#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int nclients = 0;

void printusers() {
    if (nclients) {
        printf("%d user on-line\n", nclients);
    } else {
        printf("No User on line\n");
    }
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

    char buffer[1024];
    int n;
    while ((n = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, sizeof(char), n, file);
        if (n < sizeof(buffer)) break; 
    }

    fclose(file);
    printf("Файл %s успешно получен и сохранен\n", filename);
}

void dostuff(int sock) {
    int n;
    double a, b, result;
    char buff[256];
    char op;
    int err = 0;

    n = read(sock, buff, sizeof(buff));
    if (n < 0) error("ERROR reading from socket");
    buff[n] = '\0';

    if (strncmp(buff, "file", 4) == 0) {
        n = read(sock, buff, sizeof(buff));
        if (n < 0) error("ERROR reading from socket");
        buff[n] = '\0';
        receive_file(sock, buff);
        return;
    }

    op = buff[0];

    n = read(sock, buff, sizeof(buff));
    if (n < 0) error("ERROR reading from socket");
    a = atof(buff);

    n = read(sock, buff, sizeof(buff));
    if (n < 0) error("ERROR reading from socket");
    b = atof(buff);

    result = myfunc(op, a, b, &err);

    if (err) {
        snprintf(buff, sizeof(buff), "Ошибка: неверная операция или деление на 0.\n");
    } else {
        snprintf(buff, sizeof(buff), "%f\n", result);
    }

    write(sock, buff, strlen(buff));
    nclients--; 
    printf("-disconnect\n");
    printusers();
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int pid;

    printf("TCP SERVER DEMO\n");

    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
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

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &clilen);
        if (newsockfd < 0) error("ERROR on accept");
        nclients++;

        char *client_ip = inet_ntoa(cli_addr.sin_addr);
        printf("+New connection from %s\n", client_ip);
        printusers();

        pid = fork();
        if (pid < 0) error("ERROR on fork");
        if (pid == 0) {
            close(sockfd);
            dostuff(newsockfd);
            exit(0);
        } else close(newsockfd);
    }

    close(sockfd);
    return 0;
}