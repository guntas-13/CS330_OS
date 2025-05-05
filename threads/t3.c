#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "common_threads.h"

int global = 1;

void* foo(void* arg){
    printf("&foo: %p\n", &foo);
    printf("&global: %p\n", &global);
    printf("&arg: %p\n", &arg);
    printf("arg: %p\n", arg);
    return NULL;
}

int main(int argc, char* argv[]){
    void* hmem = malloc(1);
    for (int i = 0; i < 30; i++){
        pthread_t p;
        Pthread_create(&p, NULL, foo, hmem);
//        Pthread_join(p, NULL);
    }
}
