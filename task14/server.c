#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAXLINE 1024

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, cliaddr1, cliaddr2;
    socklen_t len1, len2;
    int cli1_ready = 0, cli2_ready = 0;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        error("socket creation failed");
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr1, 0, sizeof(cliaddr1));
    memset(&cliaddr2, 0, sizeof(cliaddr2));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        error("bind failed");
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        if (!cli1_ready) {
            len1 = sizeof(cliaddr1);
            recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&cliaddr1, &len1);
            cli1_ready = 1;
            printf("Client 1 connected\n");
        }
        
        if (!cli2_ready) {
            len2 = sizeof(cliaddr2);
            recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&cliaddr2, &len2);
            cli2_ready = 1;
            printf("Client 2 connected\n");
        }

        if (cli1_ready && cli2_ready) {
            recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&cliaddr1, &len1);
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&cliaddr2, len2);

            recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&cliaddr2, &len2);
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&cliaddr1, len1);
        }
    }

    close(sockfd);
    return 0;
}