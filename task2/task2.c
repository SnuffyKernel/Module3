#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARG_COUNT 100

void parse_input(char *input, char **args) {
    int i = 0;
    args[i] = strtok(input, " \n");
    while (args[i] != NULL) {
        args[++i] = strtok(NULL, " \n");
    }
}

int main() {
    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARG_COUNT];
    pid_t pid;
    int status;

    while (1) {
        printf("command> ");
        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            perror("fgets failed");
            exit(EXIT_FAILURE);
        }

        parse_input(input, args);

        if (args[0] == NULL) {
            continue;
        }

        if (strcmp(args[0], "exit") == 0) {
            break;
        }

        pid = fork();
        if (pid == -1) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            execvp(args[0], args);
            perror("exec failed");
            exit(EXIT_FAILURE);
        } else { 
            waitpid(pid, &status, 0);
        }
    }

    return 0;
}