#include <unistd.h>

typedef struct {
    pid_t pid;  // Process ID
    int status;  // Process exit status
} Process;

void process_init(Process *p);
int process_create(Process *p, void *(*function)(void *), void *arg);
int process_wait(Process *p, int *status);
int process_run_function(Process *p, void *(*function)(void *), void *arg);