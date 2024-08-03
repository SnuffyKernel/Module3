#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg) {
    perror(msg);
    exit(0);
}

void send_file(int sock, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Не удалось открыть файл %s\n", filename);
        return;
    }

    if (send(sock, filename, strlen(filename) , 0) < 0) {
        error("Ошибка при отправке имени файла");
    }

    sleep(1);
    char buffer[1024];
    int n;
    while ((n = fread(buffer, sizeof(char), sizeof(buffer), file)) > 0) {
        if (send(sock, buffer, n, 0) < 0) {
            error("Ошибка при отправке файла");
        }
    }

    fclose(file);
    printf("Файл %s успешно отправлен\n", filename);
}

int main(int argc, char *argv[]) {
    int my_sock, portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buff[1024];

    printf("TCP DEMO CLIENT\n");

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = atoi(argv[2]);

    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock < 0) error("ERROR opening socket");

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr_list[0], (char*)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(my_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    while (1) {
        printf("Введите операцию (+, -, *, /) или 'file' для отправки файла: ");
        fgets(buff, sizeof(buff) - 1, stdin);
        send(my_sock, buff, strlen(buff), 0);

        if (strncmp(buff, "file", 4) == 0) {
            printf("Введите имя файла для отправки: ");
            fgets(buff, sizeof(buff) - 1, stdin);

            buff[strcspn(buff, "\n")] = 0; 

            send_file(my_sock, buff);
            continue;
        }

        printf("Введите первое число: ");
        fgets(buff, sizeof(buff) - 1, stdin);
        send(my_sock, buff, strlen(buff), 0);

        printf("Введите второе число: ");
        fgets(buff, sizeof(buff) - 1, stdin);
        send(my_sock, buff, strlen(buff), 0);

        int n = recv(my_sock, buff, sizeof(buff) - 1, 0);
        if (n > 0) {
            buff[n] = '\0';
            printf("Результат: %s\n", buff);
        } else {
            printf("Ошибка получения данных\n");
            break;
        }

        printf("Введите 'quit' для выхода или нажмите Enter для продолжения: ");
        fgets(buff, sizeof(buff) - 1, stdin);
        if (strcmp(buff, "quit\n") == 0) {
            printf("Выход...\n");
            break;
        }
    }

    close(my_sock);
    return 0;
}