#include "error.h"

void error(char errorname[] ) {
    char buf[256];
    int len = snprintf(buf, sizeof(buf), "\033[0;31mERROR: %s\033[0m\n", errorname);
    write(STDOUT_FILENO, buf, len);
    exit(EXIT_FAILURE);
}
