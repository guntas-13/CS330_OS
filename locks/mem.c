#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    printf("sizeof(void*) = %lu\n", sizeof(void*));
    printf("sizeof(int) = %lu\n", sizeof(int));
    printf("sizeof(double) = %lu\n", sizeof(double));
    printf("sizeof(char) = %lu\n", sizeof(char));

    return 0;
}
