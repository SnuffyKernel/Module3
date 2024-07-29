#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

#define SHM_NAME "/posix_shm_oleg"

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
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

void parent_process(shared_data *data, pid_t child_pid) {
    srand(time(NULL));

    while (running) {
        int count = rand() % 10 + 1;
        
        pthread_mutex_lock(&data->mutex);
        data->count = count;
        for (int i = 0; i < count; i++) {
            data->numbers[i] = rand() % 100;
        }
        data->ready = 1;
        pthread_cond_signal(&data->cond);
        pthread_mutex_unlock(&data->mutex);

        pthread_mutex_lock(&data->mutex);
        while (data->ready != 2 && running) {
            pthread_cond_wait(&data->cond, &data->mutex);
        }
        if (!running) {
            pthread_mutex_unlock(&data->mutex);
            break;
        }
        printf("Parent: Max = %d, Min = %d\n", data->max, data->min);
        sets_processed++;
        pthread_mutex_unlock(&data->mutex);

        sleep(1);

    }

    kill(child_pid, SIGINT);
    wait(NULL);
}

void child_process(shared_data *data) {
    while (running) {
        pthread_mutex_lock(&data->mutex);
        while (data->ready != 1 && running) {
            pthread_cond_wait(&data->cond, &data->mutex);
        }
        if (!running) {
            pthread_mutex_unlock(&data->mutex);
            break;
        }

        int max = data->numbers[0];
        int min = data->numbers[0];
        for (int i = 1; i < data->count; i++) {
            if (data->numbers[i] > max) max = data->numbers[i];
            if (data->numbers[i] < min) min = data->numbers[i];
        }
        data->max = max;
        data->min = min;
        data->ready = 2;
        pthread_cond_signal(&data->cond);
        pthread_mutex_unlock(&data->mutex);

        sleep(1);
    }
}

int main() {
    signal(SIGINT, handle_sigint);

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        error("shm_open failed");
    }

    if (ftruncate(shm_fd, sizeof(shared_data)) == -1) {
        error("ftruncate failed");
    }

    shared_data *data = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (data == MAP_FAILED) {
        error("mmap failed");
    }

    close(shm_fd);

    pthread_mutexattr_t mutex_attr;
    pthread_condattr_t cond_attr;

    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&data->mutex, &mutex_attr);
    pthread_mutexattr_destroy(&mutex_attr);

    pthread_condattr_init(&cond_attr);
    pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&data->cond, &cond_attr);
    pthread_condattr_destroy(&cond_attr);

    pid_t pid = fork();
    if (pid == -1) {
        error("fork failed");
    }

    if (pid == 0) {
        child_process(data);
    } else {
        parent_process(data, pid);
        printf("Sets processed: %d\n", sets_processed);

        wait(NULL);

        pthread_mutex_destroy(&data->mutex);
        pthread_cond_destroy(&data->cond);

        munmap(data, sizeof(shared_data));
        shm_unlink(SHM_NAME);
    }

    return 0;
}