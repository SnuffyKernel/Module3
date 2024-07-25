#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s number of random numbers\n", argv[0]);
        exit(1);
    }

    int num_numbers = atoi(argv[1]);
    if (num_numbers <= 0) {
        fprintf(stderr, "Enter a positive integer for the number.\n");
        exit(1);
    }

    int pipe_fd[2];
    pid_t pid;

    if (pipe(pipe_fd) == -1) {
        error("Pipe creation failed");
    }

    pid = fork();
    if (pid == -1) {
        error("Fork failed");
    }

    if (pid == 0) {
        close(pipe_fd[0]);

        srand(time(NULL));
        for (int i = 0; i < num_numbers; i++) {
            int random_number = rand() % 100;
            write(pipe_fd[1], &random_number, sizeof(int));
        }

        close(pipe_fd[1]);
        exit(EXIT_SUCCESS);
    } else {
        close(pipe_fd[1]);

        FILE *file = fopen("random_numbers.txt", "w");
        if (file == NULL) {
            error("File opening failed");
        }

        int received_number;
        for (int i = 0; i < num_numbers; i++) {
            read(pipe_fd[0], &received_number, sizeof(int));
            printf("Received number: %d\n", received_number);
            fprintf(file, "Received number: %d\n", received_number);
        }

        fclose(file);
        close(pipe_fd[0]);
        wait(NULL);
    }

    return 0;
}