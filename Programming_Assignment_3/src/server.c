#include "common.h"

int main(int argc, char *argv[])
{
    struct sockaddr_in    server_socket, client_socket;
    socklen_t             socket_length = sizeof(server_socket);
    ssize_t               message_length;
    int                   socket_fd;
    uint16_t              server_port = 0, OpCode;
    tftp_message_t        message;

    if (argc < 2)
    {
        printf("Please use correct format: %s [base directory] [port]\n", argv[0]);
        exit(-1);
    }

    if (argc > 2)
    {
        if (sscanf(argv[2], "%hu", &server_port))
        {
            server_port = htons(server_port);
        }
        else
        {
            fprintf(stderr, "error: Invalid port number\n");
            exit(-1);
        }
    }

    socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_fd == -1)
    {
        perror("socket():");
        exit(-1);
    }

    server_socket.sin_family = AF_INET;
    server_socket.sin_addr.s_addr = htonl(INADDR_ANY);
    server_socket.sin_port = server_port;

    if (bind(socket_fd, (struct sockaddr *) &server_socket, sizeof(server_socket)) == -1)
    {
        perror("bind():");
        close(socket_fd);
        exit(-1);
    }

    signal(SIGCHLD, (void *)zombie_handler_func);

    printf("TFTP server is running now. Listening on: %d\n", ntohs(server_socket.sin_port));

    while (1)
    {
        message_length = receive_message(socket_fd, &message, &client_socket, &socket_length);
        if (message_length < 0)
        {
            continue;
        }

        if (message_length < 4)
        {
            printf("Filename requested from %s:%d is not allowed. Please check and try again!\n", inet_ntoa(client_socket.sin_addr), ntohs(client_socket.sin_port));
            send_error_message(socket_fd, 0, "Invalid size", &client_socket, socket_length);
            continue;
        }

        OpCode = ntohs(message.uiOpcode);

        if (OpCode == RRQ_OPCODE || OpCode == WRQ_OPCODE)
        {
            // child process called to handle the incoming request
            if (fork() == 0)
            {
                handle_message(&message, message_length, &client_socket, socket_length);
                exit(-1);
            }

        }

        else
        {
            printf("Invalid request type received from %s:%d. Please check!\n", inet_ntoa(client_socket.sin_addr), ntohs(client_socket.sin_port));
            send_error_message(socket_fd, 0, "Invalid opcode", &client_socket, socket_length);
        }
    }
    close(socket_fd);
    printf("closing socket!\n");
    return 0;
}
