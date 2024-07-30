#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>


#define FILE_NAME "random_numbers.txt"
#define SEM_WRITE_NAME "/write_semaphore"
#define SEM_READ_NAME "/read_semaphore"

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void child_process(int pipe_fd[], int num_numbers) {
    close(pipe_fd[0]);

    sem_t *sem_write = sem_open(SEM_WRITE_NAME, 0);
    if (sem_write == SEM_FAILED) {
        error("sem_open write in child failed");
    }

    sem_t *sem_read = sem_open(SEM_READ_NAME, 0);
    if (sem_read == SEM_FAILED) {
        error("sem_open read in child failed");
    }

    srand(time(NULL));

    for (int i = 0; i < num_numbers; i++) {
        printf("Child waiting to access file\n");

        if (sem_wait(sem_read) == -1) {
            error("sem_wait read in child failed");
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

        if (sem_post(sem_read) == -1) {
            error("sem_post read in child failed");
        }

        int random_number = rand() % 100;
        printf("Child generated random number: %d\n", random_number);
        write(pipe_fd[1], &random_number, sizeof(int));

        sleep(1);
    }

    close(pipe_fd[1]);
    sem_close(sem_write);
    sem_close(sem_read);
    exit(0);
}

void parent_process(int pipe_fd[], int num_numbers, pid_t child_pid) {
    close(pipe_fd[1]);

    sem_t *sem_write = sem_open(SEM_WRITE_NAME, 0);
    if (sem_write == SEM_FAILED) {
        error("sem_open write in parent failed");
    }

    sem_t *sem_read = sem_open(SEM_READ_NAME, 0);
    if (sem_read == SEM_FAILED) {
        error("sem_open read in parent failed");
    }

    FILE *file = fopen(FILE_NAME, "w");
    if (file == NULL) {
        error("File opening failed in parent");
    }

    for (int i = 0; i < num_numbers; i++) {
        sleep(1);

        if (sem_wait(sem_write) == -1) {
            error("sem_wait write in parent failed");
        }

        fprintf(file, "Parent writes number %d\n", i);
        fflush(file);

        if (sem_post(sem_write) == -1) {
            error("sem_post write in parent failed");
        }

        int received_number;
        if (read(pipe_fd[0], &received_number, sizeof(int)) == -1) {
            error("Read failed in parent");
        }
        printf("Parent received number: %d\n", received_number);
        fprintf(file, "Parent received number: %d\n", received_number);
        fflush(file);
    }

    fclose(file);
    close(pipe_fd[0]);
    wait(NULL);
    sem_close(sem_write);
    sem_close(sem_read);
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

    sem_t *sem_write = sem_open(SEM_WRITE_NAME, O_CREAT | O_EXCL, 0644, 1);
    if (sem_write == SEM_FAILED) {
        error("sem_open write");
    }

    sem_t *sem_read = sem_open(SEM_READ_NAME, O_CREAT | O_EXCL, 0644, 3); // Allow up to 3 concurrent readers
    if (sem_read == SEM_FAILED) {
        error("sem_open read");
    }

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

    sem_unlink(SEM_WRITE_NAME);
    sem_unlink(SEM_READ_NAME);

    return 0;
}