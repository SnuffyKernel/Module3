#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_TEXT 512
#define QUEUE_NAME "/chat_queue"
#define PRIORITY_EXIT 10

struct message {
    char text[MAX_TEXT];
};

int main() {
    mqd_t mq;
    struct mq_attr attr;
    struct message msg;
    char buffer[MAX_TEXT];

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct message);
    attr.mq_curmsgs = 0;

    mq = mq_open(QUEUE_NAME, O_CREAT | O_WRONLY, 0644, &attr);
    if (mq == (mqd_t) -1) {
        perror("mq_open failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf(">>> ");
        fgets(buffer, MAX_TEXT, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        strcpy(msg.text, buffer);

        if (mq_send(mq, (const char*)&msg, sizeof(msg), 0) == -1) {
            perror("mq_send failed");
            exit(EXIT_FAILURE);
        }

        if (strcmp(buffer, "exit") == 0) {
            if (mq_send(mq, (const char*)&msg, sizeof(msg), PRIORITY_EXIT) == -1) {
                perror("mq_send exit failed");
                exit(EXIT_FAILURE);
            }
            break;
        }

        if (mq_receive(mq, (char*)&msg, sizeof(msg), NULL) == -1) {
            perror("mq_receive failed");
            exit(EXIT_FAILURE);
        }

        printf("Friend: %s\n", msg.text);
    }

    mq_close(mq);
    mq_unlink(QUEUE_NAME);

    return 0;
}