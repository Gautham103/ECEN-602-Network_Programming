#include "common.h"

char *work_directory;
void handle_message(tftp_message_t *message, ssize_t msglen, struct sockaddr_in *client_socket, socklen_t socket_length)
{
    int socket_fd;
    struct timeval tv;
    char *fname, *mode_finder, *end;
    FILE *fd;
    int transfermode;
    uint16_t OpCode;
    tftp_message_t client_message;
    uint8_t Data[512];
    ssize_t data_length, sndrcv;
    uint16_t Block_number = 0;
    int countdown;
    int flag = 0;

    // opening new socket to handle incoming request

    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("server: socket()");
        exit(-1);
    }

    tv.tv_usec = 0;
    tv.tv_sec = TIMEOUT;


    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        perror("setsockopt()");
        exit(-1);
    }

    printf("got a new connection from %s':%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
    fname = (char *)message->tftp_request_message.uiFileNameMode;
    end = &fname[msglen - 3];

    if (*end != '\0')
    {
        printf("invalid filename or mode requested by %s':%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
        send_error_message(socket_fd, 0, "invalid filename or mode", client_socket, socket_length);
        exit(-1);
    }

    mode_finder = strchr(fname, '\0') + 1;

    if (mode_finder > end) {
        printf("transfer mode not not given by %s:%u\n",inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
        send_error_message(socket_fd, 0, "transfer mode not given", client_socket, socket_length);
        exit(-1);
    }

    if (strncmp(fname, "../", 3) == 0 || strstr(fname, "/../") != NULL || (fname[0] == '/' && strncmp(fname, work_directory, strlen(work_directory)) != 0))
    {
        printf("file outside working directory - %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
        send_error_message(socket_fd, 0, "file outside working directory", client_socket, socket_length);
        exit(-1);
    }

    OpCode = ntohs(message->uiOpcode);
    fd = fopen(fname, OpCode == RRQ_OPCODE ? "r" : "w");

    if (fd == NULL)
    {
        perror("fopen()");
        send_error_message(socket_fd, errno, strerror(errno), client_socket, socket_length);
        exit(-1);
    }

    transfermode = strcasecmp(mode_finder, "netascii") ? NETASCII :strcasecmp(mode_finder, "octet") ? OCTET : 0;

    if (transfermode == 0)
    {
        printf("invalid transfer mode set by %s:%u\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
        send_error_message(socket_fd, 0, "invalid mode", client_socket, socket_length);
        exit(-1);
    }

    printf("File %s in mode %s will be %s by %s:%u! \n", fname, mode_finder, ntohs(message->uiOpcode) == RRQ_OPCODE ? "read" : "written", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));

    //  add netascii handling

    if (OpCode == RRQ_OPCODE)
    {
        while (!flag)
        {
            data_length = fread(Data, 1, sizeof(Data), fd);
            Block_number++;

            if (data_length < 512)
            {
                flag = 1;		// terminating data block
            }

            for (countdown = MAX_RETRIES; countdown; countdown--)
            {
                if (send_data(socket_fd, transfermode, Block_number, Data, data_length, client_socket, socket_length) < 0)
                {
                    printf("session terminated for %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                    exit(-1);
                }

                sndrcv = receive_message(socket_fd, &client_message, client_socket, &socket_length);

                if (sndrcv >= 0 && sndrcv < 4)
                {
                    printf("invalid req size received from %s:%u\n",inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                    send_error_message(socket_fd, 0, "invalid req size", client_socket, socket_length);
                    exit(-1);
                }

                if (sndrcv >= 4)
                {
                    break;
                }

                if (errno != EAGAIN)
                {
                    printf("session terminated for %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                    exit(-1);
                }

                printf("no response session from %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
            }

            if (!countdown)
            {
                printf("Transfer session has failed\n");
                exit(-1);
            }

            if (ntohs(client_message.uiOpcode) == ERROR_OPCODE)
            {
                printf("Error message %u:%s received for %s:%u\n", ntohs(client_message.tftp_error_message.uiErrorCode), client_message.tftp_error_message.uiErrorData, inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                exit(-1);
            }

            if (ntohs(client_message.uiOpcode) != ACK_OPCODE)
            {
                printf("unexpected message received from %s:%u!\n",	inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                send_error_message(socket_fd, 0, "unexpected message received", client_socket, socket_length);
                exit(-1);
            }

            if (ntohs(client_message.tftp_ack_message.uiBlockNumber) != Block_number) // size of Ack Block Number is too high
            {
                printf("Invalid Ack received from %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                send_error_message(socket_fd, 0, "invalid Ack number", client_socket, socket_length);
                exit(-1);
            }
        }
    }

    else if (OpCode == WRQ_OPCODE)
    {
        int countdown;
        int flag = 0;
        ssize_t sndrcv;
        uint16_t Block_number = 0;
        tftp_message_t client_message;

        if (send_ack(socket_fd, Block_number, client_socket, socket_length) < 0)
        {
            printf("session terminated for %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
            exit(-1);
        }

        while (!flag)
        {

            for (countdown = MAX_RETRIES; countdown; countdown--)
            {
                sndrcv = receive_message(socket_fd, &client_message, client_socket, &socket_length);

                if (sndrcv >= 0 && sndrcv < 4)
                {
                    printf("invalid req size received from %s:%u\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                    send_error_message(socket_fd, 0, "invalid req size", client_socket, socket_length);
                    exit(-1);
                }

                if (sndrcv >= 4)
                {
                    break;
                }

                if (errno != EAGAIN)
                {
                    printf("session terminated for %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                    exit(-1);
                }

                sndrcv = send_ack(socket_fd, Block_number, client_socket, socket_length);

                if (sndrcv < 0)
                {
                    printf("session terminated for %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                    exit(-1);
                }

            }

            if (!countdown)
            {
                printf("no response from %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                exit(-1);
            }

            Block_number++;

            if (sndrcv < sizeof(client_message.tftp_data_message))				//Last block in transfer session
            {
                flag = 1;
            }

            if (ntohs(client_message.uiOpcode) == ERROR_OPCODE)
            {
                printf("Error message %u:%s received for %s:%u\n", ntohs(client_message.tftp_error_message.uiErrorCode), client_message.tftp_error_message.uiErrorData, inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                exit(-1);
            }

            if (ntohs(client_message.uiOpcode) != DATA_OPCODE)
            {
                printf("unexpected data received from %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                send_error_message(socket_fd, 0, "unexpected data received", client_socket, socket_length);
                exit(-1);
            }

            if (ntohs(client_message.tftp_ack_message.uiBlockNumber) != Block_number)
            {
                printf("Invalid Block received from %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                send_error_message(socket_fd, 0, "invalid Block number", client_socket, socket_length);
                exit(-1);
            }

            sndrcv = fwrite(client_message.tftp_data_message.data, 1, sndrcv - 4, fd);

            if (sndrcv < 0)
            {
                perror("write()");
                exit(-1);
            }

            sndrcv = send_ack(socket_fd, Block_number, client_socket, socket_length);

            if (sndrcv < 0)
            {
                printf("session terminated for %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                exit(-1);
            }

        }

    }

    printf("Transfer successfully completed for %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
    fclose(fd);
    close(socket_fd);
    exit(0);
}

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

    work_directory = argv[1];

    if (chdir(work_directory) < 0)
    {
        perror("chdir()");
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
            fprintf(stderr, "error: invalid port number\n");
            exit(-1);
        }
    }

    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
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
            send_error_message(socket_fd, 0, "invalid size", &client_socket, socket_length);
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
            send_error_message(socket_fd, 0, "invalid size", &client_socket, socket_length);
        }
    }
    close(socket_fd);
    printf("closing socket!\n");
    return 0;
}
