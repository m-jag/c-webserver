#ifndef THREAD_H
#define THREAD_H
#include <pthread.h>

typedef struct {
    pthread_t thread_id;
    void *(*thread_function)(void *);
    void *args;
} ThreadHandler;

void thread_handler_init(ThreadHandler *th, void *(*thread_function)(void *), void *args);
int thread_handler_start(ThreadHandler *th);
int thread_handler_join(ThreadHandler *th);
void thread_handler_free(ThreadHandler *th);
#endif /* THREAD_H */