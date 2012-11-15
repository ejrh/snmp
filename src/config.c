#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

static void load_line(char *line, Config *config)
{
    ConfigItem *item;
    char *p;
    char *host, *oid, *freq_str;
    int freq;
    
    p = strchr(line, '#');
    if (p)
        *p = 0;
    host = strtok_r(line, " \t", &p);
    if (!host)
        return;
    oid = strtok_r(NULL, " \t", &p);
    if (!oid)
        return;
    freq_str = strtok_r(NULL, " \t", &p);
    if (!freq_str)
        return;
    
    freq = strtol(freq_str, NULL, 10);
    if (freq < 1 || freq > 60*60*24*365.24)
        return;
    
    item = malloc(sizeof(ConfigItem));
    item->host = strdup(host);
    item->host_name = NULL;
    item->port = 0;
    item->oid = strdup(oid);
    item->frequency = freq;
    item->next_time = 0;
    
    item->next = config->item_list;
    config->item_list = item;
}

Config *create_config(void)
{
    Config *config;
    
    config = malloc(sizeof(Config));
    config->item_list = NULL;
    
    return config;
}

Config *load_config(const char *filename)
{
    Config *config;
    
    FILE *f = fopen(filename, "rt");
    if (!f)
        return NULL;
    
    config = malloc(sizeof(Config));
    config->item_list = NULL;
    
    while (!feof(f))
    {
        char buf[BUFFER_SIZE];
        if (!fgets(buf, sizeof(buf), f))
             break;
        
        load_line(buf, config);
    }
    
    fclose(f);
    
    return config;
}


void print_config(Config *config, FILE *stream)
{
    ConfigItem *item = config->item_list;
    fprintf(stream, "Config:\n");
    
    while (item != NULL)
    {
        if (item->port == 0)
            fprintf(stream, "    %s %s %d\n", item->host, item->oid, item->frequency);
        else
            fprintf(stream, "    %s:%d %s %d\n", item->host, item->port, item->oid, item->frequency);
        item = item->next;
    }
}

void destroy_config(Config *config)
{
    ConfigItem *item = config->item_list;
    
    while (item != NULL)
    {
        ConfigItem *next_item = item->next;
        
        free(item->host);
        free(item->host_name);
        free(item->oid);
        free(item);
        
        item = next_item;
    }
    
    free(config);
}
