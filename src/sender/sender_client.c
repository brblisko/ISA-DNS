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

bool sender_client(parsed_params *PP) {
    int sockfd;
    char buffer[1000];
    int msg_size;
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = PP->upstream_dns_ip.s_addr;


    char packet[512] = {0};
    struct DNS_HEADER dns_header;
    struct DNS_FOOTER dns_footer;
    dns_header.id = (unsigned short) htons(12);
    dns_header.qr = 0;
    dns_header.opcode = 0;
    dns_header.aa = 0;
    dns_header.tc = 0;
    dns_header.rd = 1;
    dns_header.ra = 0;
    dns_header.z = 0;
    dns_header.rcode = 0;
    dns_header.q_count = htons(1);
    dns_header.ans_count = 0;
    dns_header.auth_count = 0;
    dns_header.add_count = 0;

    dns_footer.qtype = htons(1);
    dns_footer.qclass = htons(1);
    memcpy(packet, &dns_header, sizeof(struct DNS_HEADER));


    while ((msg_size = fread(buffer, 1, MAXLINE - 1, PP->fptr)) > 0) {
        int n, len;

        // encode
        char encoded_buffer[300];
        base32_encode((const unsigned char *) buffer, msg_size, (unsigned char *) encoded_buffer, 300);
        int encoded_buffer_size = strlen(encoded_buffer);

        // write encoded **+1 for length of encoded_buffer_size**
        packet[sizeof(struct DNS_HEADER)] = encoded_buffer_size;
        memcpy(packet + sizeof(struct DNS_HEADER) + 1, encoded_buffer, encoded_buffer_size);

        // write domain **+1 for length of base_host string**
        
        char base_host_no_dots[300] = {0};
        char helper[300] = {0};
        int char_counter = 1;
        size_t i = 0;
        for (; i < strlen(PP->base_host); i++) {
            if (PP->base_host[i] == '.') {
                base_host_no_dots[char_counter] = (!char_counter) ? 0 : i;
                //save +1 space for the number ^ and + 1 to skip dot
                memcpy(base_host_no_dots + ((!char_counter) ? 0 : char_counter + 1),
                       PP->base_host + ((!char_counter) ? 0 : char_counter + 1),
                       i + 1);
                char_counter = i + 1;
            }
        }
        base_host_no_dots[char_counter] = i;
        //save +1 space for the number ^
        memcpy(base_host_no_dots + char_counter + 1, PP->base_host + char_counter + 1, i);
        printf("%s\n", base_host_no_dots);


//        packet[sizeof(struct DNS_HEADER) + encoded_buffer_size + 1] = strlen(PP->base_host);
        memcpy(packet + sizeof(struct DNS_HEADER) + 1 + 1 + encoded_buffer_size, base_host_no_dots,
               strlen(base_host_no_dots));

        // write footer **+1 to save one \000 before footer**
        memcpy(packet + sizeof(struct DNS_HEADER) + 1 + 1 + encoded_buffer_size + strlen(PP->base_host) + 1,
               &dns_footer,
               sizeof(struct DNS_FOOTER));

        // calculate lenght
        int packet_len =
                sizeof(struct DNS_HEADER) + 1 + 1 + encoded_buffer_size + strlen(PP->base_host) + 1 +
                sizeof(struct DNS_FOOTER);

        sendto(sockfd, packet,
               packet_len, MSG_CONFIRM,
               (const struct sockaddr *) &servaddr, sizeof(servaddr));     // send data to the server


        // read the answer from the server
        n = recvfrom(sockfd, (char *) buffer, MAXLINE,
                     MSG_WAITALL, (struct sockaddr *) &servaddr,
                     &len);
        buffer[n] = '\0';
        printf("Server : %s\n", buffer);           // print the answer
    }
    close(sockfd);
    printf("* Closing the client socket ...\n");
    return true;
}
