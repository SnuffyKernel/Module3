#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define FILE_NAME "random_numbers.txt"

struct sembuf start_read = {0, 0, 0}; 
struct sembuf inc_read = {1, 1, 0}; 
struct sembuf dec_read = {1, -1, 0}; 
struct sembuf start_write = {1, 0, 0}; 
struct sembuf lock_write = {0, -1, 0}; 
struct sembuf unlock_write = {0, 1, 0}; 

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void child_process(int pipe_fd[], int num_numbers, int semid) {
    close(pipe_fd[0]);

    srand(time(NULL));

    for (int i = 0; i < num_numbers; i++) {
        printf("Child waiting to access file\n");

        if (semop(semid, &start_read, 1) == -1) {
            error("semop start_read in child failed");
        }
        if (semop(semid, &inc_read, 1) == -1) {
            error("semop inc_read in child failed");
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

        if (semop(semid, &dec_read, 1) == -1) {
            error("semop dec_read in child failed");
        }

        sleep(1);
    }

    close(pipe_fd[1]);
    exit(0);
}

void parent_process(int pipe_fd[], int num_numbers, pid_t child_pid, int semid) {
    close(pipe_fd[1]);

    FILE *file = fopen(FILE_NAME, "w");
    if (file == NULL) {
        error("File opening failed in parent");
    }

    for (int i = 0; i < num_numbers; i++) {
        sleep(2);

        if (semop(semid, &start_write, 1) == -1) {
            error("semop start_write in parent failed");
        }
        if (semop(semid, &lock_write, 1) == -1) {
            error("semop lock_write in parent failed");
        }

        fprintf(file, "Parent writes number %d\n", i);
        fflush(file);

        printf("Parent waiting for child to finish\n");

        int received_number;
        if (read(pipe_fd[0], &received_number, sizeof(int)) == -1) {
            error("Read failed in parent");
        }
        printf("Parent received number: %d\n", received_number);
        fprintf(file, "Parent received number: %d\n", received_number);
        fflush(file);

        if (semop(semid, &unlock_write, 1) == -1) {
            error("semop unlock_write in parent failed");
        }
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

    key_t key = ftok("semfile", 65);
    int semid = semget(key, 2, 0666 | IPC_CREAT);
    if (semid == -1) {
        error("semget failed");
    }

    if (semctl(semid, 0, SETVAL, 1) == -1) { 
        error("semctl SETVAL failed");
    }
    if (semctl(semid, 1, SETVAL, 0) == -1) { 
        error("semctl SETVAL failed");
    }

    if (pipe(pipe_fd) == -1) {
        error("Pipe creation failed");
    }

    pid = fork();
    if (pid == -1) {
        error("Fork failed");
    }
    if (pid == 0) {
        child_process(pipe_fd, num_numbers, semid);
    } else {
        parent_process(pipe_fd, num_numbers, pid, semid);
    }

    if (semctl(semid, 0, IPC_RMID) == -1) {
        error("semctl IPC_RMID failed");
    }

    return 0;
}