#include "parameter_parser.h"

bool parse_parameters(int argc, char **argv, parsed_params *PP) {
    if (argc != 3) {
        fprintf(stderr, "ERROR - missing arguments\n");
        return false;
    }
    PP->base_host = argv[1];
    PP->dst_filepath = argv[2];

    return true;
}
