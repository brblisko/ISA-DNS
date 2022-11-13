#include "parameter_parser.h"
#include "sender_client.h"

int main(int argc, char **argv) {
    struct parsed_params PP;

    if (!parse_parameters(argc, argv, &PP)) {
        return 1;
    }

    if (!sender_client(&PP)) {
        fclose(PP.fptr);
        return 1;
    }

    printf("\n\n\nbase_host: %s\n", PP.base_host);
    printf("dst_filepath: %s\n", PP.dst_filepath);
    printf("upstream_dns_ip: %s\n", inet_ntoa(PP.upstream_dns_ip));
    fclose(PP.fptr);
    return 0;
}
