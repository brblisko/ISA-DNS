#ifndef ISA_DNS_SENDER_CLIENT_H
#define ISA_DNS_SENDER_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <uuid/uuid.h>
#include "parameter_parser.h"

#define PORT        53
#define MAXLINE     32
#define PATH_ID     5
#define DATA_ID     13
#define FINISH_ID   15
#define ERROR_ID    100


bool sender_client(parsed_params *PP);

#endif //ISA_DNS_SENDER_CLIENT_H
