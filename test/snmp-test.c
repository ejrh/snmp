#include "../src/snmp.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>


static SNMPMessage *make_request()
{
    SNMPMessage *message = snmp_create_message();
    
    snmp_set_version(message, 0);
    snmp_set_community(message, "private");
    snmp_set_pdu_type(message, SNMP_GET_REQUEST_TYPE);
    snmp_set_request_id(message, 1);
    snmp_set_error(message, 0);
    snmp_set_error_index(message, 0);
    
    return message;
}


static void test_create_and_parse()
{
    SNMPMessage *message = make_request();
    SNMPMessage *message2;
    int len;
    unsigned char *buf;
    int i;
    
    snmp_add_varbind_null(message, "1.3.6.1.4.1.2680.1.2.7.3.2.0");
    
    snmp_print_message(message, stdout);
    
    len = snmp_message_length(message);
    buf = malloc(len);
    snmp_render_message(message, buf);
    
    snmp_destroy_message(message);
    
    printf("%02x", buf[0]);
    for (i = 1; i < len; i++)
        printf(" %02x", buf[i]);
    printf("\n");
    
    message2 = snmp_parse_message(buf, len);
    snmp_print_message(message2, stdout);
    snmp_destroy_message(message2);
    
    free(buf);
}

static void test_timeticks()
{
    SNMPMessage *message = make_request();
    SNMPMessage *message2;
    int len;
    unsigned char *buf;
    
    char *oid_str;
    int type;
    int int_value;
    
    snmp_add_varbind_integer_type(message, "1.3.6.1.2.1.1.3.0", SNMP_TIMETICKS_TYPE, 42);
    
    len = snmp_message_length(message);
    buf = malloc(len);
    snmp_render_message(message, buf);
    
    snmp_destroy_message(message);
    
    message2 = snmp_parse_message(buf, len);
    
    snmp_get_varbind_integer(message, 0, &oid_str, &type, &int_value);
    printf("Timeticks (type 0x%02x) = %d\n", type, int_value);
    
    snmp_destroy_message(message2);
    
    free(buf);
}

int main()
{
    test_create_and_parse();
    test_timeticks();
    return 0;
}
