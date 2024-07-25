#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

#define FILE_NAME "random_numbers.txt"

volatile sig_atomic_t file_accessible = 0;

void handle_sigusr1(int sig) {
    file_accessible = 0; 
    printf("Child received SIGUSR1, blocking file access\n");
}

void handle_sigusr2(int sig) {
    file_accessible = 1;
    printf("Child received SIGUSR2, allowing file access\n");
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void child_process(int pipe_fd[], int num_numbers) {
    close(pipe_fd[0]);

    signal(SIGUSR1, handle_sigusr1);
    signal(SIGUSR2, handle_sigusr2);

    srand(time(NULL));

    for (int i = 0; i < num_numbers; i++) {
        printf("Child waiting for SIGUSR2 to read file\n");
        pause();

        while (!file_accessible) { 
            printf("Child waiting for file to be accessible\n");
            pause();
        }

        FILE *file = fopen(FILE_NAME, "r");
        if (file == NULL) {
            error("File opening failed in child");
        }

        char buffer[256];
        while (fgets(buffer, sizeof(buffer), file)) {
            printf("Child read from file: %s", buffer);
        }
        fclose(file);

        int random_number = rand() % 100;
        printf("Child generated random number: %d\n", random_number);
        write(pipe_fd[1], &random_number, sizeof(int));

        sleep(1);
    }

    close(pipe_fd[1]); 
    exit(0);
}

void parent_process(int pipe_fd[], int num_numbers, pid_t child_pid) {
    close(pipe_fd[1]); 

    FILE *file = fopen(FILE_NAME, "w");
    if (file == NULL) {
        error("File opening failed in parent");
    }

    for (int i = 0; i < num_numbers; i++) {
        printf("Parent writing to file\n");
        fprintf(file, "Parent writes number %d\n", i);
        fflush(file);

        printf("Parent sending SIGUSR2 to child\n");
        kill(child_pid, SIGUSR2); 

        int received_number;
        read(pipe_fd[0], &received_number, sizeof(int));
        printf("Parent received number: %d\n", received_number);
        fprintf(file, "Parent received number: %d\n", received_number);
        fflush(file);

        printf("Parent sending SIGUSR1 to child\n");
        kill(child_pid, SIGUSR1);
    }

    fclose(file);
    close(pipe_fd[0]); 
    wait(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_random_numbers>\n", argv[0]);
        exit(1);
    }

    int num_numbers = atoi(argv[1]);
    if (num_numbers <= 0) {
        fprintf(stderr, "Please enter a positive integer for the number of random numbers.\n");
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
        child_process(pipe_fd, num_numbers);
    } else { 
        parent_process(pipe_fd, num_numbers, pid);
    }

    return 0;
}