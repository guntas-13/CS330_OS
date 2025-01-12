#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "common.h"
#include "common_threads.h"

volatile int counter = 0;
int loops;

void* worker(void* arg) 
{
    int i;
    for (i = 0; i < loops; i++)
    {
        counter = counter + 1;
    }
    return NULL;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "usage: threads <loops>\n");
        exit(1);
    }
    loops = atoi(argv[1]);
    pthread_t p1, p2;
    
    printf("Initial Value of Counter: %d\n", counter);

    Pthread_create(&p1, NULL, worker, NULL);
    Pthread_create(&p2, NULL, worker, NULL);

    Pthread_join(p1, NULL);
    Pthread_join(p2, NULL);

    printf("Final Value of Counter: %d\n", counter);

    return 0;
}
