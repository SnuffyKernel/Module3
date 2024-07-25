#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_TEXT 512

struct message {
    long msg_type;
    char text[MAX_TEXT];
};

int main() {
    key_t key;
    int msgid;
    struct message msg;
    char buffer[MAX_TEXT];

    key = ftok("progfile", 65);

    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf(">>> ");
        fgets(buffer, MAX_TEXT, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        msg.msg_type = 1;
        strcpy(msg.text, buffer);

        if (msgsnd(msgid, &msg, sizeof(msg.text), 0) == -1) {
            exit(EXIT_FAILURE);
        }

        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        if (msgrcv(msgid, &msg, sizeof(msg.text), 2, 0) == -1) {
            exit(EXIT_FAILURE);
        }

        printf("Friend: %s\n", msg.text);
    }

    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}