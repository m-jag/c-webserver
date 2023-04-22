#include <stdio.h>
#include <stdlib.h>

#include "thread.h"

void thread_handler_init(ThreadHandler *th, void *(*thread_function)(void *), void *args) {
    th->thread_function = thread_function;
    th->args = args;
}

int thread_handler_start(ThreadHandler *th) {
    int result = pthread_create(&(th->thread_id), NULL, th->thread_function, th->args);
    if (result != 0) {
        perror("pthread_create");
    }
    return result;
}

int thread_handler_join(ThreadHandler *th) {
    int result = pthread_join(th->thread_id, NULL);
    if (result != 0) {
        perror("pthread_join");
    }
    return result;
}

void thread_handler_free(ThreadHandler *th) {
    if (th->args)
    {
        free(th->args);
    }
}