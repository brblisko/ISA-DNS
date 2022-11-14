#include <sys/stat.h>
#include <signal.h>
#include "receiver_client.h"
#include "dns_receiver_events.h"
#include "../base32.h"

FILE *fp = NULL;
int sockfd = 0;
char file_path[300] = {0};

struct DNS_HEADER {
    unsigned short id; // identification number

    unsigned char rd: 1; // recursion desired
    unsigned char tc: 1; // truncated message
    unsigned char aa: 1; // authoritive answer
    unsigned char opcode: 4; // purpose of message
    unsigned char qr: 1; // query/response flag

    unsigned char rcode: 4; // response code
    unsigned char cd: 1; // checking disabled
    unsigned char ad: 1; // authenticated data
    unsigned char z: 1; // its z! reserved
    unsigned char ra: 1; // recursion available

    unsigned short q_count; // number of question entries
    unsigned short ans_count; // number of answer entries
    unsigned short auth_count; // number of authority entries
    unsigned short add_count; // number of resource entries
};


// from stackoverflow
// https://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix
static void _mkdir(const char *dir) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, 0777);
            *p = '/';
        }
    mkdir(tmp, 0777);
}

bool process_file_path(char *data, parsed_params *PP) {
    char decoded[300] = {0};
    base32_decode((const unsigned char *) data, (unsigned char *) decoded, 300);
    char helper[300] = {0};
    memcpy(helper, decoded, 300);


    char *token, *last;
    last = token = strtok(helper, "/");
    for (; (token = strtok(NULL, "/")) != NULL; last = token);


    char file_path_no_file[300] = {0};
    memset(file_path, 0, 300);

    memcpy(file_path_no_file, PP->dst_filepath, strlen(PP->dst_filepath));
    file_path_no_file[strlen(PP->dst_filepath)] = '/';
    if (strcmp(decoded, last) != 0) {
        memcpy(file_path_no_file + strlen(PP->dst_filepath) + 1, decoded, strlen(decoded) - strlen(last) - 1);
    }

    memcpy(file_path, PP->dst_filepath, strlen(PP->dst_filepath));
    file_path[strlen(PP->dst_filepath)] = '/';
    memcpy(file_path + strlen(PP->dst_filepath) + 1, decoded, strlen(decoded));

    _mkdir(file_path_no_file);

    fp = fopen(file_path, "wb");
    if (fp == NULL) {
        return false;
    }

    return true;
}


void extract(char *buffer, char *base_host_name, char *data, int *id) {
    int to_skip = (int) buffer[12];
    int position_in_buffer = 12;
    int position_in_base = 0;
    bool first = true;

    struct DNS_HEADER *dns_header = (struct DNS_HEADER *) buffer;
    *id = ntohs(dns_header->id);

    while (to_skip != 0) {
        if (first) {
            first = false;

            memcpy(data, buffer + position_in_buffer + 1, to_skip);

            position_in_buffer += to_skip + 1;
            to_skip = (int) buffer[position_in_buffer];

            memcpy(base_host_name + position_in_base, buffer + position_in_buffer + 1, to_skip);

            position_in_base += to_skip;
            position_in_buffer += to_skip + 1;

            to_skip = (int) buffer[position_in_buffer];

        } else {
            memcpy(base_host_name + position_in_base + 1, buffer + position_in_buffer + 1, to_skip);
            base_host_name[position_in_base] = '.';
            position_in_base += to_skip + 1;
            position_in_buffer += to_skip + 1;

            to_skip = (int) buffer[position_in_buffer];
        }
    }
    base_host_name[position_in_base] = '\0';

}

void release(void) {
    if (fp != NULL) {
        fclose(fp);
    }
    if (sockfd != 0) {
        close(sockfd);
    }
}

//stackoverflow
//https://stackoverflow.com/questions/4217037/catch-ctrl-c-in-c
void INThandler(int sig) {

    signal(sig, SIG_IGN);

    release();

    exit(0);

}

void change_id_to_error(char *buffer) {
    struct DNS_HEADER *dns_header = (struct DNS_HEADER *) buffer;
    dns_header->id = (unsigned short) htons(ERROR_ID);
}


bool receiver_client(parsed_params *PP) {
    signal(SIGINT, INThandler);

    int msg_size, sent_data_size;
    char buffer[MAXLINE];
    char data[MAXLINE] = {0};
    int id;
    char base_host_name[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t length;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "ERROR - socket creation failed\n");
        return false;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);


    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *) &servaddr,
             sizeof(servaddr)) < 0) {
        fprintf(stderr, "ERROR - bind failed\n");
        return false;
    }
    length = sizeof(cliaddr);

    int counter = 0;
    while ((msg_size = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *) &cliaddr, &length)) >= 0) {

        memset(data, 0, MAXLINE);
        extract(buffer, base_host_name, data, &id);

        if (strcmp(base_host_name, PP->base_host) == 0) {

            if (id == PATH_ID) {

                dns_receiver__on_transfer_init(&(cliaddr.sin_addr));

                if (!process_file_path(data, PP)) {
                    change_id_to_error(buffer);
                }

            } else if (id == DATA_ID) {
                if (fp != NULL) {
                    counter++;
                    char decoded[300] = {0};
                    char glued[MAXLINE] = {0};

                    memcpy(glued, data, strlen(data));
                    glued[strlen(data)] = '.';
                    memcpy(glued + strlen(data) + 1, base_host_name, strlen(base_host_name));

                    int real_size = base32_decode((const unsigned char *) data, (unsigned char *) decoded, 300);

                    dns_receiver__on_chunk_received(&(cliaddr.sin_addr), file_path, counter, real_size);

                    dns_receiver__on_query_parsed(file_path, glued);

                    fwrite(decoded, sizeof(char), real_size, fp);
                }

            } else if (id == FINISH_ID) {
                int file_size = 0;
                fseek(fp, 0L, SEEK_END);
                file_size = ftell(fp);

                dns_receiver__on_transfer_completed(file_path, file_size);

                if (fp != NULL) {
                    fclose(fp);
                    fp = NULL;
                }

                counter = 0;
            }
        } else {
            continue;
        }

        sent_data_size = sendto(sockfd, buffer, msg_size, 0, (struct sockaddr *) &cliaddr, length); // send the answer
        if (sent_data_size == -1) {
            fprintf(stderr, "ERROR - sendto() failed\n");
            release();
            return false;
        } else if (sent_data_size != msg_size) {
            fprintf(stderr, "ERROR - send(): buffer written partially\n");
            release();
            return false;
        }
    }

    close(sockfd);

    return true;
}
