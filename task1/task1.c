#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void calculate_areas(int *sides, int start, int end, const char *process_name) {
    for (int i = start; i < end; ++i) {
        int area = sides[i] * sides[i];
        printf("%s: Side length: %d. Square area: %d\n", process_name, sides[i], area);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s side length\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int num_sides = argc - 1;
    int sides[num_sides];

    for (int i = 0; i < num_sides; ++i) {
        sides[i] = atoi(argv[i + 1]);
        if (sides[i] <= 0) {
            fprintf(stderr, "Side lengths should be positive integers.\n");
            exit(EXIT_FAILURE);
        }
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { 
        calculate_areas(sides, 0, num_sides / 2, "Child process");
        exit(EXIT_SUCCESS);
    } else {
        calculate_areas(sides, num_sides / 2, num_sides, "Parent process");
        wait(NULL);
    }

    return 0;
}