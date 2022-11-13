#include "parameter_parser.h"
#include "receiver_client.h"

int main(int argc, char **argv) {
    struct parsed_params PP;

    if (!parse_parameters(argc, argv, &PP)) {
        return 1;
    }

    if (!receiver_client(&PP)) {
        return 1;
    }

    printf("\n\n\nbase_host: %s\n", PP.base_host);
    printf("dst_filepath: %s\n", PP.dst_filepath);
    return 0;
}
