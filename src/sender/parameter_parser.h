#include <stdio.h>
#include <stdbool.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#ifndef ISA_DNS_PARAM_PARSER_H
#define ISA_DNS_PARAM_PARSER_H

typedef struct parsed_params {
    struct in_addr upstream_dns_ip;
    char *base_host;
    char *dst_filepath;
    FILE *fptr;
} parsed_params;

bool parse_parameters(int argc, char **argv, parsed_params *PP);

#endif //ISA_DNS_PARAM_PARSER_H
