#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "common.h"
#include "common_threads.h"

volatile int counter = 0;
int loops;

pthread_mutex_t mutex; 

void* worker(void* arg) 
{
    int i;
    for (i = 0; i < loops; i++)
    {
        Mutex_lock(&mutex);
        counter = counter + 1;
        Mutex_unlock(&mutex);
    }
    return NULL;
}

int main(int argc, char** argv)
{
    Mutex_init(&mutex);
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

    pthread_mutex_destroy(&mutex);
    return 0;
}
