#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    printf("The address of main is: %p\n", (void*)main);
    printf("The address of heap is: %p\n", (void*)malloc(1));
    int x = 3;
    printf("The address of stack is: %p\n", (void*)&x);
    return 0;
}
