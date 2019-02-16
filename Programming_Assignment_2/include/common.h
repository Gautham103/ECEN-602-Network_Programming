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
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/select.h>
#include <sys/time.h>

#define QUEUE_SIZE 50
#define BUFFER_SIZE 20
#define PAYLOAD_SIZE 512

#define JOIN 2
#define SEND 4
#define FWD 3
#define ACK 7
#define NAK 5
#define ONLINE 8
#define OFFLINE 6
#define IDLE 9
#define STDIN fileno(stdin)

/* SBCP message header format */
typedef struct
{
    unsigned int uiVrsn:9;
    unsigned int uiType:7;
    unsigned int uiLength:16;
}sbcp_header_t;

/* SBCP message attribute formate */
typedef struct
{
    unsigned int uiType:16;
    unsigned int uiLength:16;
    char         acPayload[PAYLOAD_SIZE];
}sbcp_attribute_t;

/* SBCP message structure */
struct message
{
    sbcp_header_t sMsgHeader;
    sbcp_attribute_t sMsgAttribute;
};


int create_socket();
void set_server_address(struct sockaddr_in *server_address, int port);
void bind_server(int socket_fd, struct sockaddr_in server_address);
void start_listening(int socket_fd);
int iReadLine (int iSocket_fd, char *pcBuffer, int iBufferSize);
int writen (int iSocket_fd, char *pcBuffer, int iBufferSize);
void zombie_handler_func(int signum);
void read_write(int connect_fd);

#endif // COMMON_INCLUDED