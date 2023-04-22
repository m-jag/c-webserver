#include "webserver.h"
#include "process.h"
#include "thread.h"
#include <errno.h>
#include <sys/un.h>

int main(int argc, char** argv) {
    // Fork the process
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) {
        // Child process
        child_process();
    }
    else {
        // Parent process
        main_process();
    }

    return 0;
}

void *request_handler(void *arg) {
    RequestHandlerArgs *args = (RequestHandlerArgs *)arg;
    int socket_fd = args->socket_fd;
    char buffer[BUFFER_SIZE] = {0};
    char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nHello World!";
    ssize_t read_bytes;

    read_bytes = read(socket_fd, buffer, BUFFER_SIZE);
    if (read_bytes > 0) {
        printf("Request received:\n%s\n", buffer);

        // Spawn child process to handle request
        Process p;
        if (process_create(&p, handle_request, &socket_fd) == -1) {
            fprintf(stderr, "Error creating child process to handle request\n");
            close(socket_fd);
            return NULL;
        }

        // Wait for child process to complete
        int status;
        if (process_wait(&p, &status) == -1) {
            fprintf(stderr, "Error waiting for child process to complete\n");
            close(socket_fd);
            return NULL;
        }

        // Check child process exit status
        if (WIFEXITED(status)) {
            int child_status = WEXITSTATUS(status);
            if (child_status != 0) {
                fprintf(stderr, "Child process exited with status %d\n", child_status);
            }
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "Child process terminated by signal %d\n", WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            fprintf(stderr, "Child process stopped by signal %d\n", WSTOPSIG(status));
        } else {
            fprintf(stderr, "Unexpected status returned from waitpid: %d\n", status);
        }

        // Send response and close socket
        write(socket_fd, response, strlen(response));
        close(socket_fd);
        printf("Response sent, connection closed.\n");
    }

    return NULL;
}

void *handle_request(void *arg) {
    RequestHandlerArgs *args = (RequestHandlerArgs *)arg;
    int socket_fd = args->socket_fd;
    printf("handle_request: %d\n", socket_fd);
    char buffer[BUFFER_SIZE] = {0};
    char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nHello World!";
    ssize_t read_bytes;

    read_bytes = read(socket_fd, buffer, BUFFER_SIZE);
    if (read_bytes > 0) {
        printf("Request received:\n%s\n", buffer);
        write(socket_fd, response, strlen(response));

        printf("Response sent, connection closed.\n");
    }
    else
    {
        printf("No bytes read\n");
    }

    if (socket_fd >= 0)
    {
        close(socket_fd);
    }
    return NULL;
}

void main_process() {
    int child_socket_fd = -1, server_fd = -1, new_socket = -1;
    struct sockaddr_in address;
    socklen_t address_length;
    int addrlen = sizeof(address);
    
    
    // Open a socket to write filedescriptors to children
    int server_socket_fd = -1;
    server_socket_fd = create_main_server_socket();
    if (server_socket_fd < 0)
    {
        perror("Failed to open socket to send file descriptors");
        goto CLEANUP;
    }

    // Connect to child processes
    for (int i = 0; i < CHILD_COUNT; i++)
    {
        child_socket_fd = accept(server_socket_fd, (struct sockaddr *) &address,&address_length);
    }

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        goto CLEANUP;
    }

    // Set server address and port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind server socket to address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        goto CLEANUP;
    }

    // Listen for incoming connections
    if (listen(server_fd, MAX_THREADS) < 0) {
        perror("listen failed");
        goto CLEANUP;
    }

    // Accept incoming connections and handle requests in separate threads
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            goto CLEANUP;
        }

        //sending the file descriptor
        printf("From send_fd %d \n",send_fd(child_socket_fd, new_socket));
        

        printf("Sent message: socket_fd = %d\n", new_socket);
    }

CLEANUP:
    // Note: Don't close new_socket since the child needs it
    // TODO: Revisit this understanding

    // Close the unix socket for sharing sockets (server)
    if (server_socket_fd >= 0)
    {
        close(server_socket_fd);
        server_socket_fd = -1;
    }
    
    // Close the unix socket for sharing sockets (child)
    if (child_socket_fd >= 0)
    {
        close(child_socket_fd);
        child_socket_fd = -1;
    }

    // Close server socket
    if (server_fd >= 0)
    {
        close(server_fd);
    }
}


void child_process() {
    ThreadHandler th[MAX_THREADS];
    // Parent process: receiver
    printf("Receiver process started.\n");
    
    // Extract the RequestHandlerArgs from the message       
    // Allocate arguments and init thread_handler
    RequestHandlerArgs *args = malloc(sizeof(RequestHandlerArgs));

    // Open a socket to read client filedescriptors from
    int child_socket_fd = -1;
    child_socket_fd = create_child_server_socket();
    if (child_socket_fd < 0)
    {
        perror("Failed to open socket to recieve file descriptors");
        goto CLEANUP;
    }

    // Accept incoming connections and handle requests in separate threads
    for (int i = 0; i < MAX_THREADS; i++) {

        args->socket_fd = recv_fd(child_socket_fd);
        if (args->socket_fd < 0)
        {
            perror("recieve invalid file descriptor");
            goto CLEANUP;
        }

        thread_handler_init(&th[i], handle_request, args);
        
        printf("Received message: socket_fd = %d\n", args->socket_fd);

        int result = thread_handler_start(&th[i]);
        if (result != 0) {
            perror("thread_handler_start");
            goto CLEANUP;
        }
    }

CLEANUP:    
    // Wait for all threads to complete
    for (int i = 0; i < MAX_THREADS; i++) {
        // Thread not used
        if (th[i].thread_id == 0)       
        {
            continue;
        }

        int result = thread_handler_join(&th[i]);
        if (result != 0) {
            perror("thread_handler_join");
            // continue to try and join other threads
            continue;
        }
    }

    // Release thread resources
    for (int i = 0; i < MAX_THREADS; i++) {
        // Thread not used
        if (th[i].thread_id == 0)       
        {
            continue;
        }

        // Close the accepted socket (the socket sent from the main process)
        if (th[i].args && ((RequestHandlerArgs *)th[i].args)->socket_fd >= 0)
        {
            close(((RequestHandlerArgs *)th[i].args)->socket_fd);
        }
        
        // Free arguments allocated for thread
        if (th[i].args)
        {
            memset(th[i].args, 0, sizeof(RequestHandlerArgs));
            free(th[i].args);
        }
    }

    // Close the unix socket for sharing sockets (recieving from main process)
    if (child_socket_fd >= 0)
    {
        close(child_socket_fd);
        child_socket_fd = -1;
    }
}

int create_main_server_socket()
{
    struct sockaddr_un address;
    int socket_fd = -1;

    socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if(socket_fd < 0)
    {
        printf("socket() failed\n");
        goto ERROR;
    }

    unlink("./demo_socket");

    /* start with a clean address structure */
    memset(&address, 0, sizeof(struct sockaddr_un));

    address.sun_family = AF_UNIX;
    snprintf(address.sun_path, sizeof(address.sun_path)-1, "./demo_socket");

    if(bind(socket_fd, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) != 0)
    {
        printf("bind() failed\n");
        goto ERROR;
    }

    if(listen(socket_fd, 5) != 0)
    {
        printf("listen() failed\n");
        goto ERROR;
    }

    goto CLEANUP;

ERROR:
    if (socket_fd >= 0)
    {
        close(socket_fd);
        socket_fd = -1;
    }

CLEANUP:
    return socket_fd;
}

int create_child_server_socket()
{
    struct sockaddr_un address;
    int  socket_fd = -1;

    socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if(socket_fd < 0)
    {
        printf("socket() failed\n");
        goto ERROR;
    }

    /* start with a clean address structure */
    memset(&address, 0, sizeof(struct sockaddr_un));

    address.sun_family = AF_UNIX;
    snprintf(address.sun_path, sizeof(address.sun_path)-1, "./demo_socket");

    if(connect(socket_fd, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) != 0)
    {
        printf("connect() failed\n");
        goto ERROR;
    }
    goto CLEANUP;

ERROR:
    if (socket_fd >= 0)
    {
        close(socket_fd);
        socket_fd = -1;
    }

CLEANUP:
    return socket_fd;
}

int send_fd(int fd, int fd_to_send)
{
    int temp;
    struct iovec    iov[1];
    struct msghdr   msg;
    char            buf[2]; /* send_fd()/recv_fd() 2-byte protocol */

    iov[0].iov_base = buf;
    iov[0].iov_len  = 2;
    msg.msg_iov     = iov;
    msg.msg_iovlen  = 1;
    msg.msg_name    = NULL;
    msg.msg_namelen = 0;
    if (fd_to_send < 0) {
        msg.msg_control    = NULL;
        msg.msg_controllen = 0;
        buf[1] = -fd_to_send;   /* nonzero status means error */
        if (buf[1] == 0)
            buf[1] = 1; /* -256, etc. would screw up protocol */
    } else {
        if (cmptr == NULL && (cmptr = malloc(CONTROLLEN)) == NULL)
            return(-1);
        cmptr->cmsg_level  = SOL_SOCKET;
        cmptr->cmsg_type   = SCM_RIGHTS;
        cmptr->cmsg_len    = CONTROLLEN;
        msg.msg_control    = cmptr;
        msg.msg_controllen = CONTROLLEN;
        *(int *)CMSG_DATA(cmptr) = fd_to_send;     /* the fd to pass */
        buf[1] = 0;          /* zero status means OK */
    }
    buf[0] = 0;              /* null byte flag to recv_fd() */
    printf("before sendmsg \n");
    if ((temp = sendmsg(fd, &msg, 0)) != 2)
    {
        printf("inside sendmsg condition %d\n",temp);
        return(-1);  
    }
    printf("after sendmsg %d\n",temp);
    return(0);

}

int recv_fd(int fd)
{
  int             newfd, nr, status;
  char            *ptr;
  char            buf[MAXLINE];
  struct iovec    iov[1];
  struct msghdr   msg;

  status = -1;
  for ( ; ; ) 
  {
    iov[0].iov_base = buf;
    iov[0].iov_len  = sizeof(buf);
    msg.msg_iov     = iov;
    msg.msg_iovlen  = 1;
    msg.msg_name    = NULL;
    msg.msg_namelen = 0;
    if (cmptr == NULL && (cmptr = malloc(CONTROLLEN)) == NULL)
      return(-1);
    msg.msg_control    = cmptr;
    msg.msg_controllen = CONTROLLEN;
    if ((nr = recvmsg(fd, &msg, 0)) < 0) 
    {
      printf("recvmsg errrrror %d %d %s\n",nr,errno,strerror(errno));
    //   perror("recvmsg errrrror");
    } else if (nr == 0) 
    {
      perror("connection closed by server");
      return(-1);
    }
    /*
    * See if this is the final data with null & status.  Null
    * is next to last byte of buffer; status byte is last byte.
    * Zero status means there is a file descriptor to receive.
    */
    for (ptr = buf; ptr < &buf[nr]; ) 
    {
      if (*ptr++ == 0) 
      {
        if (ptr != &buf[nr-1])
          perror("message format error");
        status = *ptr & 0xFF;  /* prevent sign extension */
        if (status == 0) 
        {
          if (msg.msg_controllen != CONTROLLEN)
          perror("status = 0 but no fd");
          newfd = *(int *)CMSG_DATA(cmptr);
        } else 
        {
          newfd = -status;
        }
        nr -= 2;
      }
    }
    if (nr > 0)
      return(-1);
    if (status >= 0)    /* final data has arrived */
      return(newfd);  /* descriptor, or -status */
  }
}