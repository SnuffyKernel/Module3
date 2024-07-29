#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define FILE_NAME "random_numbers.txt"

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void child_process(int pipe_fd[], int num_numbers, int semid) {
    close(pipe_fd[0]);

    srand(time(NULL));

    struct sembuf sem_op;

    for (int i = 0; i < num_numbers; i++) {

        printf("Child waiting to access file\n");

        sem_op.sem_num = 0;
        sem_op.sem_op = -1;
        sem_op.sem_flg = 0;
        if (semop(semid, &sem_op, 1) == -1) {
            error("semop P in child failed");
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

        sem_op.sem_num = 0;
        sem_op.sem_op = 1;
        sem_op.sem_flg = 0;
        if (semop(semid, &sem_op, 1) == -1) {
            error("semop V in child failed");
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

    struct sembuf sem_op;

    for (int i = 0; i < num_numbers; i++) {

        sleep(2);

        sem_op.sem_num = 0;
        sem_op.sem_op = 1; 
        sem_op.sem_flg = 0;
        
        if (semop(semid, &sem_op, 1) == -1) {
            error("semop V in parent failed");
        }

        fprintf(file, "Parent writes number %d\n", i);
        fflush(file);

        printf("Parent waiting for child to finish\n");

        sem_op.sem_num = 0;
        sem_op.sem_op = -1;
        sem_op.sem_flg = 0;
        if (semop(semid, &sem_op, 1) == -1) {
            error("semop P in parent failed");
        }

        int received_number;
        if(read(pipe_fd[0], &received_number, sizeof(int)) == -1) {
            error("Read failed in parent");
        }
        printf("Parent received number: %d\n", received_number);
        fprintf(file, "Parent received number: %d\n", received_number);
        fflush(file);

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
    int semid = semget(key, 1, 0666 | IPC_CREAT);
    if (semid == -1) {
        error("semget failed");
    }

    if (semctl(semid, 0, SETVAL, 0) == -1) {
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