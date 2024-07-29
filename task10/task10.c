#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
#include <semaphore.h>

#define FILE_NAME "random_numbers.txt"

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void child_process(int pipe_fd[], int num_numbers, sem_t *sem_parent, sem_t *sem_child) {
    close(pipe_fd[0]);

    srand(time(NULL));

    for (int i = 0; i < num_numbers; i++) {

        printf("Child waiting for file to be accessible\n");
        sem_wait(sem_child);

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

        sem_post(sem_parent);

        sleep(1);
    }

    close(pipe_fd[1]);
    exit(0);
}

void parent_process(int pipe_fd[], int num_numbers, pid_t child_pid, sem_t *sem_parent, sem_t *sem_child) {
    close(pipe_fd[1]);

    FILE *file = fopen(FILE_NAME, "w");
    if (file == NULL) {
        error("File opening failed in parent");
    }

    for (int i = 0; i < num_numbers; i++) {
         sleep(1);
        fprintf(file, "Parent writes number %d\n", i);
        fflush(file);

        sem_post(sem_child);
        sem_wait(sem_parent); 

        int received_number;
        read(pipe_fd[0], &received_number, sizeof(int));
        printf("Parent received number: %d\n", received_number);
        fprintf(file, "Parent received number: %d\n", received_number);
        fflush(file);

        sleep(1);
    }

    fclose(file);
    close(pipe_fd[0]);
    wait(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s num\n", argv[0]);
        exit(1);
    }

    int num_numbers = atoi(argv[1]);
    if (num_numbers <= 0) {
        fprintf(stderr, "Please enter a positive integer for the number of random numbers.\n");
        exit(1);
    }

    int pipe_fd[2];
    pid_t pid;
    sem_t sem_parent, sem_child;

    if (pipe(pipe_fd) == -1) {
        error("Pipe creation failed");
    }

    if (sem_init(&sem_parent, 1, 0) == -1) {
        error("Semaphore initialization failed");
    }

    if (sem_init(&sem_child, 1, 0) == -1) {
        error("Semaphore initialization failed");
    }

    pid = fork();
    if (pid == -1) {
        error("Fork failed");
    }
    if (pid == 0) { 
        child_process(pipe_fd, num_numbers, &sem_parent, &sem_child);
    } else { 
        parent_process(pipe_fd, num_numbers, pid, &sem_parent, &sem_child);
    }

    sem_destroy(&sem_parent);
    sem_destroy(&sem_child);

    return 0;
}