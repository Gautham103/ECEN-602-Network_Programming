#ifndef COMMON_INCLUDED
#define COMMON_INCLUDED

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>

#define QUEUE_SIZE 50
#define BUFFER_SIZE 1024

int create_socket();
void set_server_address(struct sockaddr_in *server_address, int port);
void bind_server(int socket_fd, struct sockaddr_in server_address);
void start_listening(int socket_fd);
int iReadLine (int iSocket_fd, char *pcBuffer, int iBufferSize);
int writen (int iSocket_fd, char *pcBuffer, int iBufferSize);
void zombie_handler_func(int signum);
void read_write(int connect_fd);

#endif // COMMON_INCLUDED
