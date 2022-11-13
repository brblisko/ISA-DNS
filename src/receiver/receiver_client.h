#ifndef ISA_DNS_RECEIVER_CLIENT_H
#define ISA_DNS_RECEIVER_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "parameter_parser.h"

#define PORT     53
#define MAXLINE 1024

bool receiver_client(parsed_params *PP);

#endif //ISA_DNS_RECEIVER_CLIENT_H
