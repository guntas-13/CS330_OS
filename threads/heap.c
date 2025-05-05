#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int *p = (int *)malloc(10);
    int *q = (int *)malloc(10);

    printf("%p\n", p);
    printf("%p\n", q);
    free(p);
    free(q);
    return 0;
}
