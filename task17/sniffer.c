#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#define BUFFER_SIZE 65536

void process_packet(unsigned char *buffer, int size);

int main() {
    int raw_sock;
    int data_size;
    struct sockaddr saddr;
    socklen_t saddr_size;
    unsigned char *buffer = (unsigned char *)malloc(BUFFER_SIZE);

    raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw_sock < 0) {
        perror("Ошибка создания сырого сокета");
        return 1;
    }

    saddr_size = sizeof(saddr);

    while (1) {
        data_size = recvfrom(raw_sock, buffer, BUFFER_SIZE, 0, &saddr, &saddr_size);
        if (data_size < 0) {
            perror("Ошибка получения пакета");
            return 1;
        }

        process_packet(buffer, data_size);
    }

    close(raw_sock);
    return 0;
}

void process_packet(unsigned char *buffer, int size) {
    struct iphdr *ip_header = (struct iphdr *)(buffer);
    struct udphdr *udp_header = (struct udphdr *)(buffer + ip_header->ihl * 4);
    unsigned char *data = buffer + ip_header->ihl * 4 + sizeof(struct udphdr);

    int header_size = ip_header->ihl * 4 + sizeof(struct udphdr);
    int data_size = size - header_size;

    printf("\n[+] Захвачен пакет:\n");
    printf("   |-IP адрес источника        : %s\n", inet_ntoa(*(struct in_addr *)&ip_header->saddr));
    printf("   |-IP адрес назначения       : %s\n", inet_ntoa(*(struct in_addr *)&ip_header->daddr));
    printf("   |-Порт источника            : %d\n", ntohs(udp_header->source));
    printf("   |-Порт назначения           : %d\n", ntohs(udp_header->dest));
    printf("   |-Размер данных             : %d\n", data_size);

    printf("   |-Данные:\n");
    for (int i = 0; i < data_size; i++) {
        printf("%c", data[i]);
    }
    printf("\n");
}