
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include "net.h"
#include "snmp.h"
#include "config.h"
#include "sqlite-logging.h"

#define BUFLEN 65535

#define DEFAULT_LISTEN_PORT 12345

#define DEFAULT_AGENT_PORT 161

#define DEFAULT_CONFIG_FILENAME "poller.conf"

typedef struct Options
{
    int verbose;
    int listen_port;
    char *config_filename;
    char *log_filename;
    
    Config *config;
    LogContext *log_context;
} Options;

static void parse_args(int argc, char *argv[], Options *options)
{
    int c;
    
    options->verbose = 0;
    options->listen_port = DEFAULT_LISTEN_PORT;
    options->config_filename = DEFAULT_CONFIG_FILENAME;
    options->log_filename = NULL;

    opterr = 1;

    while ((c = getopt (argc, argv, "vp:c:l:")) != -1)
        switch (c)
        {
            case 'v':
                options->verbose = 1;
                break;
            case 'p':
                options->listen_port = strtol(optarg, NULL, 0);
                break;
            case 'c':
                options->config_filename = optarg;
                break;
            case 'l':
                options->log_filename = optarg;
                break;
            default:
                exit(1);
        }

    if (options->listen_port < 1 || options->listen_port > 65535)
    {
        fprintf(stderr, "Listen port must be between 1 and 65535\n");
        exit(1);
    }
    
    options->config = NULL;
    options->log_context = NULL;
}

void get_time_str(char *buf, int size)
{
    time_t time_buf;
    struct tm tm_buf;
    
    time(&time_buf);
    localtime_r(&time_buf, &tm_buf);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", &tm_buf);    
}

static void log_message(Options *options, SNMPMessage *message, char *sender_host)
{
    char *host_str = sender_host;
    char timestamp_str[20];
    char *oid_str;
    char *value_str;
    int i = 0;
    
    get_time_str(timestamp_str, sizeof(timestamp_str));
    
    while (snmp_get_varbind_as_string(message, i, &oid_str, NULL, &value_str))
    {
        if (options->log_context != NULL)
            sqlite_log(options->log_context, host_str, timestamp_str, oid_str, value_str);
        else
            printf("%s\t%s\t%s\t%s\n", host_str, timestamp_str, oid_str, value_str);
        
        i++;
    }
}

static unsigned int next_request_id = 0;

unsigned int send_request(Options *options, int socket, char *agent_host, int agent_port, char *oid)
{
    SNMPMessage *message;
    int len;
    unsigned char *buf;
    unsigned long int request_id = next_request_id++;
  
    message = snmp_create_message();
    snmp_set_version(message, 0);
    snmp_set_community(message, "public");
    snmp_set_pdu_type(message, SNMP_GET_REQUEST_TYPE);
    snmp_set_request_id(message, request_id);
    snmp_set_error(message, 0);
    snmp_set_error_index(message, 0);
    snmp_add_varbind_null(message, oid);
    
    len = snmp_message_length(message);
    buf = malloc(len);
    snmp_render_message(message, buf);
    
    if (options->verbose)
        snmp_print_message(message, stderr);
    
    snmp_destroy_message(message);
    
    if (options->verbose)
        fprintf(stderr, "Sending datagram to %s:%d\n", agent_host, agent_port);

    send_udp_datagram(buf, len, socket, agent_host, agent_port);
    
    free(buf);
    
    return request_id;
}
        
static void check_requests(Options *options, int socket)
{
    ConfigItem *item = options->config->item_list;
    
    while (item != NULL)
    {
        item->wait--;
        
        if (item->wait <= 0)
        {
            send_request(options, socket, item->host_name, item->port, item->oid);
            item->wait = item->frequency;
        }
        
        item = item->next;
    }
}

static void check_for_responses(Options *options, int socket)
{
    while (1)
    {
        char buf[BUFLEN];
        SNMPMessage *message;
        char *sender_host;
        int sender_port;
        
        int nr = receive_udp_datagram(buf, BUFLEN, socket, &sender_host, &sender_port);
        
        if (nr == 0)
            break;
        
        if (options->verbose)
            fprintf(stderr, "Received packet from %s:%d\n", 
                    sender_host, sender_port);
        
        message = snmp_parse_message(buf, nr);
        
        if (options->verbose)
            snmp_print_message(message, stderr);
        
        if (snmp_get_pdu_type(message) == SNMP_GET_RESPONSE_TYPE)
            log_message(options, message, sender_host);
        
        snmp_destroy_message(message);
    }
}

static void initialise_config(Config *config)
{
    ConfigItem *item = config->item_list;
    
    while (item != NULL)
    {
        if (!split_host_port(item->host, DEFAULT_AGENT_PORT, &item->host_name, &item->port))
        {
            fprintf(stderr, "Warning: Agent host cannot be parsed: %s\n", item->host);
            item->port = 0;
            continue;
        }
        else if (item->port < 1 || item->port > 65535)
        {
            fprintf(stderr, "Agent port must be between 1 and 65535: %s\n", item->host);
            item->port = 0;
            continue;
        }
        
        item->wait = 0;
        
        item = item->next;
    }
}

static volatile sig_atomic_t reload_config = 1;

static void handle_sigquit(int signum)
{
    reload_config = 1;
}

static void run(Options *options)
{
    int socket = open_udp_socket(options->listen_port);

    if (options->verbose)
        fprintf(stderr, "Opened socket on port %d\n", options->listen_port);
    
    if (signal(SIGQUIT, handle_sigquit) == SIG_IGN)
        signal(SIGQUIT, SIG_IGN);
    
    reload_config = 1;
    
    while (1)
    {
        if (reload_config)
        {
            if (options->config != NULL)
                destroy_config(options->config);
            if (options->log_context != NULL)
                destroy_sqlite_context(options->log_context);
            
            options->config = load_config(options->config_filename);
            if (!options->config)
            {
                fprintf(stderr, "Warning: Config file cannot be read: %s\n", options->config_filename);
                options->config = create_config();
            }
            initialise_config(options->config);
            
            if (options->verbose)
            {
                fprintf(stderr, "Loaded config from %s\n", options->config_filename);
                print_config(options->config, stderr);
            }
            
            if (options->log_filename != NULL)
            {
                options->log_context = create_sqlite_context(options->log_filename);
            }
            
            reload_config = 0;
        }
        
        check_requests(options, socket);
        sleep(1);
        check_for_responses(options, socket);
    }
    
    close(socket);
}

int main(int argc, char *argv[])
{
    Options options;
    
    parse_args(argc, argv, &options);
    run(&options);
    return 0;
}
