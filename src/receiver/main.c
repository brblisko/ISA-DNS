#include "parameter_parser.h"

int main(int argc, char **argv) {
    struct parsed_params PP;

    if (!parse_parameters(argc, argv, &PP)) {
        return 1;
    }

    return 0;
}
