#ifndef WEBSERVER_H
#define WEBSERVER_H
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CONN 5
#define REQUESTS_PER_PROCESS 10
#define MAX_THREADS 10

typedef struct {
    int socket_fd;
} RequestHandlerArgs;

typedef struct {
    long mtype;
    int socket_fd;
} message;

#define MSG_SIZE sizeof(message)

void *handle_request(void *args);
void *request_handler(void *arg);
void child_process();

#define CHILD_COUNT 1
void main_process();
int create_main_server_socket();
int create_child_server_socket();

#define CONTROLLEN  CMSG_LEN(sizeof(int))
static struct cmsghdr   *cmptr = NULL;  /* malloc'ed first time */
int send_fd(int fd, int fd_to_send);

#define MAXLINE 10
int recv_fd(int fd);

#endif /* WEBSERVER_H */