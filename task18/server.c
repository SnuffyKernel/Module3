#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

double myfunc(char op, double a, double b, int *error) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (b != 0) return a / b;
            else {
                *error = 1;
                return 0;
            }
        default: 
            *error = 1;
            return 0;
    }
}

void receive_file(int sock, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Не удалось создать файл %s\n", filename);
        return;
    }

    char buffer[BUFFER_SIZE];
    int n;
    memset(buffer,0,sizeof(buffer));
    while ((n = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, sizeof(char), n, file);
        if (n < sizeof(buffer)) break;
    }

    fclose(file);
    printf("Файл %s успешно получен и сохранен\n", filename);
}

void handle_client(int sock) {
    int n;
    double a, b, result;
    char buffer[BUFFER_SIZE];
    char op;
    int err = 0;

    while(1) {
        n = read(sock, buffer, sizeof(buffer));
        if (n <= 0) return;
        buffer[n] = '\0';

        if (strncmp(buffer, "file", 4) == 0) {
            n = recv(sock, buffer, sizeof(buffer), 0);
            if (n <= 0) return;
            buffer[n] = '\0';
            printf("%s\n", buffer);
            receive_file(sock, buffer);
            return;
        }

        op = buffer[0];

        n = read(sock, buffer, sizeof(buffer));
        if (n <= 0) return;
        a = atof(buffer);

        n = read(sock, buffer, sizeof(buffer));
        if (n <= 0) return;
        b = atof(buffer);

        result = myfunc(op, a, b, &err);

        if (err) {
            snprintf(buffer, sizeof(buffer), "Ошибка: неверная операция или деление на 0.\n");
        } else {
            snprintf(buffer, sizeof(buffer), "%f\n", result);
        }

        write(sock, buffer, strlen(buffer));
    }
}

int main(int argc, char *argv[]) {
    int sockfd, portno;
    struct sockaddr_in serv_addr;

    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(EXIT_FAILURE);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    bzero((char*) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, MAX_CLIENTS);

    fd_set active_fds, read_fds;
    FD_ZERO(&active_fds);
    FD_SET(sockfd, &active_fds);
    int max_fd = sockfd;

    while (1) {
        read_fds = active_fds;

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0)
            error("ERROR on select");

        for (int i = 0; i <= max_fd; ++i) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == sockfd) {
                    int new_sockfd;
                    struct sockaddr_in cli_addr;
                    socklen_t clilen = sizeof(cli_addr);
                    new_sockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &clilen);
                    if (new_sockfd < 0)
                        error("ERROR on accept");
                    FD_SET(new_sockfd, &active_fds);
                    if (new_sockfd > max_fd)
                        max_fd = new_sockfd;
                    printf("Новое соединение от %s\n", inet_ntoa(cli_addr.sin_addr));
                } else {

                    handle_client(i);
                    close(i);
                    FD_CLR(i, &active_fds);
                }
            }
        }
    }
    close(sockfd);
    return 0;
}