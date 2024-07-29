#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>

#define SHM_KEY 12345
#define SEM_KEY 54321

typedef struct {
    int numbers[100];
    int count;
    int max;
    int min;
    int ready;
} shared_data;

volatile sig_atomic_t sets_processed = 0;
volatile sig_atomic_t running = 1;

void handle_sigint(int sig) {
    running = 0;
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void parent_process(int shmid, int semid, pid_t child_pid) {
    shared_data *data = (shared_data *)shmat(shmid, NULL, 0);
    if (data == (void *) -1) {
        error("shmat failed in parent");
    }

    struct sembuf sem_op;
    srand(time(NULL));

    while (running) {
        int count = rand() % 10 + 1;
        data->count = count;
        for (int i = 0; i < count; i++) {
            data->numbers[i] = rand() % 100;
        }

        data->ready = 1;

        sem_op.sem_num = 0;
        sem_op.sem_op = 1;
        sem_op.sem_flg = 0;
        if (semop(semid, &sem_op, 1) == -1) {
            error("semop V in parent failed");
        }

        sem_op.sem_num = 1;
        sem_op.sem_op = -1;
        sem_op.sem_flg = 0;
        if (semop(semid, &sem_op, 1) == -1) {
            error("semop P in parent failed");
        }

        printf("Parent: Max = %d, Min = %d\n", data->max, data->min);
        sets_processed++;
        sleep(1);
    }

    kill(child_pid, SIGINT);
    wait(NULL);

    shmdt(data);
}

void child_process(int shmid, int semid) {
    shared_data *data = (shared_data *)shmat(shmid, NULL, 0);
    if (data == (void *) -1) {
        error("shmat failed in child");
    }

    struct sembuf sem_op;

    while (running) {
        sem_op.sem_num = 0;
        sem_op.sem_op = -1;
        sem_op.sem_flg = 0;
        if (semop(semid, &sem_op, 1) == -1) {
            error("semop P in child failed");
        }

        if (!running) break;

        int max = data->numbers[0];
        int min = data->numbers[0];
        for (int i = 1; i < data->count; i++) {
            if (data->numbers[i] > max) max = data->numbers[i];
            if (data->numbers[i] < min) min = data->numbers[i];
        }
        data->max = max;
        data->min = min;
        data->ready = 2;

        sem_op.sem_num = 1;
        sem_op.sem_op = 1;
        sem_op.sem_flg = 0;
        if (semop(semid, &sem_op, 1) == -1) {
            error("semop V in child failed");
        }
    }

    shmdt(data);
}

int main() {
    signal(SIGINT, handle_sigint);

    int shmid = shmget(SHM_KEY, sizeof(shared_data), 0666 | IPC_CREAT);
    if (shmid == -1) {
        error("shmget failed");
    }

    int semid = semget(SEM_KEY, 2, 0666 | IPC_CREAT);
    if (semid == -1) {
        error("semget failed");
    }

    if (semctl(semid, 0, SETVAL, 0) == -1 || semctl(semid, 1, SETVAL, 0) == -1) {
        error("semctl SETVAL failed");
    }

    pid_t pid = fork();
    if (pid == -1) {
        error("fork failed");
    }

    if (pid == 0) {
        child_process(shmid, semid);
    } else {
        parent_process(shmid, semid, pid);
        printf("Sets processed: %d\n", sets_processed);

        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);
    }

    return 0;
}