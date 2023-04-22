#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "process.h"

void process_init(Process *p) {
    p->pid = -1;
    p->status = -1;
}

int process_create(Process *p, void *(*function)(void *), void *arg) {
    p->pid = fork();
    if (p->pid == -1) {
        perror("fork");
        return -1;
    } else if (p->pid == 0) {
        // Child process
        void *result = function(arg);
        exit((intptr_t)result);
    } else {
        // Parent process
        return 0;
    }
}



int process_wait(Process *p, int *status) {
    int result = waitpid(p->pid, status, 0);
    if (result == -1) {
        perror("waitpid");
        return -1;
    } else {
        return 0;
    }
}

int process_run_function(Process *p, void *(*function)(void *), void *arg) {
    int status;
    int child_status;

    // Create process
    if (process_create(p, function, arg) == -1) {
        return -1;
    }

    // Wait for process to complete
    if (process_wait(p, &status) == -1) {
        return -1;
    }

    // Check child process exit status
    if (WIFEXITED(status)) {
        child_status = WEXITSTATUS(status);
        if (child_status != 0) {
            fprintf(stderr, "Child process exited with status %d\n", child_status);
            return -1;
        }
    } else if (WIFSIGNALED(status)) {
        fprintf(stderr, "Child process terminated by signal %d\n", WTERMSIG(status));
        return -1;
    } else if (WIFSTOPPED(status)) {
        fprintf(stderr, "Child process stopped by signal %d\n", WSTOPSIG(status));
        return -1;
    } else {
        fprintf(stderr, "Unexpected status returned from waitpid: %d\n", status);
        return -1;
    }

    return 0;
}
