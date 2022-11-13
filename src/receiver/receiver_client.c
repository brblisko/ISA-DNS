#include <ctype.h>
#include "receiver_client.h"

bool receiver_client(parsed_params *PP) {
    int sockfd;
    int n, r;
    char buffer[MAXLINE];
    char *hello = "Hello from server";
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

    while ((n = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *) &cliaddr, &length)) >= 0) {
        printf("data received from %s, port %d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

        for (r = 0; r < n; r++)
            if (islower(buffer[r]))
                buffer[r] = toupper(buffer[r]);
            else if (isupper(buffer[r]))
                buffer[r] = tolower(buffer[r]);

        r = sendto(sockfd, buffer, n, 0, (struct sockaddr *) &cliaddr, length); // send the answer
        if (r == -1) {
            fprintf(stderr, "ERROR - sendto() failed\n");
            return false;
        } else if (r != n) {
            fprintf(stderr, "ERROR - send(): buffer written partially\n");
            return false;
        } else {
            printf("data \"%.*s\" sent to %s, port %d\n", r - 1, buffer, inet_ntoa(cliaddr.sin_addr),
                   ntohs(cliaddr.sin_port));
        }
    }

    printf("closing socket\n");
    close(sockfd);

    return true;
}
