#include "sender_client.h"
#include "../base32.h"

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

struct DNS_FOOTER {
    unsigned short qtype;
    unsigned short qclass;
};


void encode_base_host(char *base_host, char *base_host_no_dots) {
    int char_counter = 0;
    size_t i = 0;
    for (; i < strlen(base_host); i++) {
        if (base_host[i] == '.') {
            base_host_no_dots[char_counter] = i - char_counter;
            memcpy(base_host_no_dots + char_counter + 1, base_host + char_counter, i - char_counter);
            char_counter = i + 1;
        }
    }
    base_host_no_dots[char_counter] = i - char_counter;
    memcpy(base_host_no_dots + char_counter + 1, base_host + char_counter, i - char_counter);
}

void prepare_dns_structs(struct DNS_HEADER *dns_header, struct DNS_FOOTER *dns_footer, int id) {
    dns_header->id = (unsigned short) htons(id);
    dns_header->qr = 0;
    dns_header->opcode = 0;
    dns_header->aa = 0;
    dns_header->tc = 0;
    dns_header->rd = 1;
    dns_header->ra = 0;
    dns_header->z = 0;
    dns_header->rcode = 0;
    dns_header->q_count = htons(1);
    dns_header->ans_count = 0;
    dns_header->auth_count = 0;
    dns_header->add_count = 0;

    dns_footer->qtype = htons(1);
    dns_footer->qclass = htons(1);
}

int prepare_packet(char *packet,
                   struct DNS_HEADER *dns_header,
                   char *msg,
                   char *base_host,
                   struct DNS_FOOTER *dns_footer) {
    //add header
    memcpy(packet, dns_header, sizeof(struct DNS_HEADER));
    //add data
    packet[sizeof(struct DNS_HEADER)] = strlen(msg);
    memcpy(packet + sizeof(struct DNS_HEADER) + 1, msg, strlen(msg));
    // add base_host
    memcpy(packet + sizeof(struct DNS_HEADER) + 1 + strlen(msg), base_host,
           strlen(base_host));
    // add footer
    memcpy(packet + sizeof(struct DNS_HEADER) + 1 + strlen(msg) + strlen(base_host) + 1,
           dns_footer,
           sizeof(struct DNS_FOOTER));
    return sizeof(struct DNS_HEADER) + 1 + strlen(msg) + strlen(base_host) + 1 +
           sizeof(struct DNS_FOOTER);
}

void clean(char *packet, char *encoded_buffer) {
    memset(packet, 0, 512);
    memset(encoded_buffer, 0, 300);
}

bool sender_client(parsed_params *PP) {
    int sockfd;
    char buffer[1000];
    char packet[512] = {0};
    int msg_size;
    struct sockaddr_in servaddr;
    char base_host_no_dots[300] = {0};
    char encoded_buffer[300] = {0};
    struct DNS_HEADER dns_header;
    struct DNS_FOOTER dns_footer;
    int packet_len;
    int n, len;


    encode_base_host(PP->base_host, base_host_no_dots);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = PP->upstream_dns_ip.s_addr;

    //send path
    prepare_dns_structs(&dns_header, &dns_footer, PATH_ID);
    base32_encode((const unsigned char *) PP->dst_filepath, strlen(PP->dst_filepath), (unsigned char *) encoded_buffer,
                  300);
    packet_len = prepare_packet(packet, &dns_header, encoded_buffer, base_host_no_dots, &dns_footer);

    sendto(sockfd, packet,
           packet_len, MSG_CONFIRM,
           (const struct sockaddr *) &servaddr, sizeof(servaddr));     // send data to the server


    n = recvfrom(sockfd, (char *) buffer, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *) &servaddr,
                 &len);
    buffer[n] = '\0';

    clean(packet, encoded_buffer);


    while ((msg_size = fread(buffer, sizeof(char), MAXLINE, PP->fptr)) > 0) {
        // encode
        base32_encode((const unsigned char *) buffer, msg_size, (unsigned char *) encoded_buffer, 300);

        prepare_dns_structs(&dns_header, &dns_footer, DATA_ID);
        packet_len = prepare_packet(packet, &dns_header, encoded_buffer, base_host_no_dots, &dns_footer);

        sendto(sockfd, packet,
               packet_len, MSG_CONFIRM,
               (const struct sockaddr *) &servaddr, sizeof(servaddr));     // send data to the server


        clean(packet, encoded_buffer);

        // read the answer from the server
        n = recvfrom(sockfd, (char *) buffer, MAXLINE,
                     MSG_WAITALL, (struct sockaddr *) &servaddr,
                     &len);
        buffer[n] = '\0';
        printf("Server : %s\n", buffer);           // print the answer
    }

    //send finito
    prepare_dns_structs(&dns_header, &dns_footer, FINISH_ID);
    base32_encode((const unsigned char *) "finished", strlen("finished"), (unsigned char *) encoded_buffer,
                  300);
    packet_len = prepare_packet(packet, &dns_header, encoded_buffer, base_host_no_dots, &dns_footer);

    sendto(sockfd, packet,
           packet_len, MSG_CONFIRM,
           (const struct sockaddr *) &servaddr, sizeof(servaddr));     // send data to the server

    n = recvfrom(sockfd, (char *) buffer, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *) &servaddr,
                 &len);
    buffer[n] = '\0';

    clean(packet, encoded_buffer);


    close(sockfd);
    printf("* Closing the client socket ...\n");
    return true;
}
