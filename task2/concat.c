#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s str\n", argv[0]);
        return 1;
    }

    char result[1024] = "";
    for (int i = 1; i < argc; ++i) {
        strcat(result, argv[i]);
    }

    printf("Concatenated string: %s\n", result);
    return 0;
}