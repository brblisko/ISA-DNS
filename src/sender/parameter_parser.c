#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "parameter_parser.h"

bool process_upstream_dns_ip(char *ip_string, parsed_params *PP) {
    if (inet_aton(ip_string, &PP->upstream_dns_ip) == 0) {
        fprintf(stderr, "ERROR - invalid upstream dns ip\n");
        return false;
    }
    return true;
}

bool load_default_nameserver(char **ip_string, char *line) {
    FILE *fp = popen("cat /etc/resolv.conf | grep \"nameserver\" ", "r");
    if (fp == NULL) {
        fprintf(stderr, "ERROR - can't open file with dns configuration\n");
        return false;
    }

    fgets(line, 99, fp);
    *ip_string = strtok(line, " ");
    *ip_string = strtok(NULL, " ");

    pclose(fp);
    return true;
}

bool parse_parameters(int argc, char **argv, parsed_params *PP) {
    PP->fptr = NULL;

    if (argc == 3) {
        PP->base_host = argv[1];
        PP->dst_filepath = argv[2];

    } else if (argc == 4) {

        PP->base_host = argv[1];
        PP->dst_filepath = argv[2];
        PP->fptr = fopen(argv[3], "r");
        if (PP->fptr == NULL) {
            fprintf(stderr, "ERROR - unable to read file %s\n", argv[3]);
            return false;
        }

    } else if (argc == 5) {
        if (strcmp(argv[1], "-u") == 0) {
            if (!process_upstream_dns_ip(argv[2], PP)) {
                return false;
            }
        }
        PP->base_host = argv[3];
        PP->dst_filepath = argv[4];

    } else if (argc == 6) {
        if (strcmp(argv[1], "-u") == 0) {
            if (!process_upstream_dns_ip(argv[2], PP)) {
                return false;
            }
        }
        PP->base_host = argv[3];
        PP->dst_filepath = argv[4];
        PP->fptr = fopen(argv[5], "r");
        if (PP->fptr == NULL) {
            fprintf(stderr, "ERROR - unable to read file %s\n", argv[5]);
            return false;
        }

    } else {
        fprintf(stderr, "ERROR - missing arguments\n");
        return false;
    }

    if (strcmp(argv[1], "-u") != 0) {
        char *ip_string = NULL;
        char helper_line[100];
        if (!load_default_nameserver(&ip_string, helper_line)) {
            return false;
        }
        if (!process_upstream_dns_ip(ip_string, PP)) {
            return false;
        }
    }

    if (PP->fptr == NULL) {
        PP->fptr = stdin;
    }
    return true;
}
