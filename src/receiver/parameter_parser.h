#include <stdio.h>
#include <stdbool.h>

#ifndef ISA_DNS_PARAMETER_PARSER_H
#define ISA_DNS_PARAMETER_PARSER_H


typedef struct parsed_params {
    char *base_host;
    char *dst_filepath;
} parsed_params;

bool parse_parameters(int argc, char **argv, parsed_params *PP);

#endif //ISA_DNS_PARAMETER_PARSER_H
