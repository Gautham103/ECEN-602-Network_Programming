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
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#define QUEUE_SIZE 50
#define BUFFER_SIZE 20
#define FILE_SIZE 512

#define TIMEOUT 4
#define MAX_RETRIES 10

#define RRQ_OPCODE 1
#define WRQ_OPCODE 2
#define DATA_OPCODE 3
#define ACK_OPCODE 4
#define ERROR_OPCODE 5

#define NETASCII 1
#define OCTET 2
typedef union tftp_message tftp_message_t;
union tftp_message{
    uint16_t uiOpcode;

    struct{
    	uint16_t uiOpcode;
    	uint8_t uiFileNameMode[FILE_SIZE];
    }tftp_request_message;

    struct{
    	uint16_t uiOpcode;
    	uint16_t uiBlockNumber;
    }tftp_ack_message;

    struct{
    	uint16_t uiOpcode;
    	uint16_t uiBlockNumber;
    	uint8_t data[FILE_SIZE];
    }tftp_data_message;

    struct{
    	uint16_t uiOpcode;
    	uint16_t uiErrorCode;
    	uint8_t uiErrorData[FILE_SIZE];
    }tftp_error_message;
};

int create_socket(bool isIPv4);
void set_server_address(struct sockaddr_in *server_address, char * ip, int port);
void set_server_address_ipv6(struct sockaddr_in6 *server_address, char * ip, int port);
void bind_server(int socket_fd, struct sockaddr_in server_address);
void bind_server_ipv6(int socket_fd, struct sockaddr_in6 server_address);
void start_listening(int socket_fd);
int accept_connection(struct sockaddr_in * client_addresses, int client_count, int socket_fd);
ssize_t send_message(int write_fd, tftp_message_t * message, size_t size, struct sockaddr_in *socket_addr, socklen_t socket_len);
void update_message(char * message, int index, int new_line);
ssize_t send_data(int write_fd, int mode, uint16_t uiBlockNumber, uint8_t *data, ssize_t message_length,
        struct sockaddr_in * address, socklen_t socket_len);
ssize_t send_ack(int write_fd,uint16_t uiBlockNumber, struct sockaddr_in * address, socklen_t socket_len);
ssize_t send_error_message(int write_fd, int error_code, char * error_data, struct sockaddr_in * address, socklen_t socket_len);
ssize_t receive_message(int receive_fd, tftp_message_t * recv_message, struct sockaddr_in * address, socklen_t *socket_len);
void zombie_handler_func(int signum);
#endif // COMMON_INCLUDED
