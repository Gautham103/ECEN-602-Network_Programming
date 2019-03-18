#include "common.h"

int create_socket(bool isIPv4)
{
    int socket_fd = -1;
    if (isIPv4 ==  true)
    {
        socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    }
    else
    {
        socket_fd = socket(AF_INET6, SOCK_STREAM, 0);
    }
    if(socket_fd == -1)
    {
        perror("Socket Create Failed");
        exit(-1);
    }
    else{
        printf("Socket is Created Successfully....\n");
    }
    return socket_fd;
}

void set_server_address(struct sockaddr_in *server_address, char * ip, int port)
{
    bzero(server_address, sizeof(*server_address));
    (*server_address).sin_family = AF_INET;
    (*server_address).sin_addr.s_addr = inet_addr(ip);
    (*server_address).sin_port = htons(port);
}

void set_server_address_ipv6(struct sockaddr_in6 *server_address, char * ip, int port)
{
    bzero(server_address, sizeof(*server_address));
    (*server_address).sin6_family = AF_INET6;
    if (inet_pton(AF_INET6, ip, &(*server_address).sin6_addr) <= 0)
        printf("inet_pton error for %s", ip);
    (*server_address).sin6_port = htons(port);
}

void bind_server(int socket_fd, struct sockaddr_in server_address)
{
    int val = bind(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address));
    if(val != 0)
    {
        perror("Socket Bind Failed");
        exit(-1);
    }
    else
    {
        printf("Socket is binded Successfully....\n");
    }
}

void bind_server_ipv6(int socket_fd, struct sockaddr_in6 server_address)
{
    int val = bind(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address));
    if(val != 0)
    {
        perror("Socket Bind Failed");
        exit(-1);
    }
    else
    {
        printf("Socket is binded Successfully....\n");
    }
}

void start_listening(int socket_fd)
{
    int val = listen(socket_fd, QUEUE_SIZE);
    if(val != 0)
    {
        perror("Listening Failed");
        exit(-1);
    }
    else
    {
        printf("Server Listening Started....\n");
    }
}

int accept_connection(struct sockaddr_in * client_addresses, int client_count, int socket_fd)
{
    socklen_t len = (socklen_t)sizeof(client_addresses[client_count]);
    int new_socket_fd = accept(socket_fd, (struct sockaddr *)&client_addresses[client_count], &len);
    if(new_socket_fd < 0)
    {
        perror("Accept connection error: ");
        exit(-1);
    }
    return new_socket_fd;
}

ssize_t send_message(int write_fd, tftp_message_t * message, size_t size, struct sockaddr_in *socket_addr, socklen_t socket_len){
	ssize_t data_sent = sendto(write_fd, message, size, 0, (struct sockaddr *) socket_addr, socket_len);
	if(data_sent < 0)
	{
		perror("Error Sending Data:");
	}
	return data_sent;
}

FILE * create_temp_file(FILE *fd, char * temp_file_name){
    srand(time(0));
    int c = rand();
    sprintf(temp_file_name, "temp_file_%d.txt", c);
    FILE * temp_fd = fopen(temp_file_name, "w");
    int ch;
    int nextChar = -1;
    while(1){
        ch = getc(fd);
        if(nextChar >= 0){
            fputc(nextChar, temp_fd);
            nextChar = -1;
        }
        if(ch == EOF){
            if(ferror(temp_fd)){
                perror("Error writing to Temp File");
            }
            break;
        }
        else if(ch == '\n'){
            ch = '\r';
            nextChar = '\n';
        }
        else if(ch == '\r'){
            nextChar = '\0';
        }
        else{
            nextChar = -1;
        }
        fputc(ch,temp_fd);
    }
    fclose(temp_fd);
    return fopen(temp_file_name, "r");
}

ssize_t send_data(int write_fd, int mode, uint16_t uiBlockNumber, uint8_t *data, ssize_t message_length, struct sockaddr_in * address, socklen_t socket_len){
	char msg_body[FILE_SIZE];
	tftp_message_t data_message;
	int extra_char = 0;
	int total_len = 0;
    memcpy(msg_body, data, message_length);
	total_len = extra_char+message_length;
	data_message.uiOpcode = htons(DATA_OPCODE);
	data_message.tftp_data_message.uiBlockNumber = htons(uiBlockNumber);
	memcpy(data_message.tftp_data_message.data, msg_body, total_len);
	return send_message(write_fd, &data_message, total_len+4, address, socket_len);
}

ssize_t send_ack(int write_fd,uint16_t uiBlockNumber, struct sockaddr_in * address, socklen_t socket_len)
{
	tftp_message_t ack_message;
	ack_message.uiOpcode = htons(ACK_OPCODE);
	ack_message.tftp_ack_message.uiBlockNumber = htons(uiBlockNumber);
	return send_message(write_fd, &ack_message, sizeof(ack_message.tftp_ack_message), address, socket_len);
}

ssize_t send_error_message(int write_fd, int error_code, char * error_data, struct sockaddr_in * address, socklen_t socket_len)
{
	tftp_message_t error_msg;
	if(strlen(error_data) >= FILE_SIZE){
		return 0;
	}
	error_msg.uiOpcode = htons(ERROR_OPCODE);
	error_msg.tftp_error_message.uiErrorCode = htons(error_code);
	strcpy((char *)error_msg.tftp_error_message.uiErrorData, error_data);
	return send_message(write_fd, &error_msg, 5+strlen(error_data), address, socket_len);
}

ssize_t receive_message(int receive_fd, tftp_message_t * recv_message, struct sockaddr_in * address, socklen_t * socket_len){
	ssize_t recv_msg_size = recvfrom(receive_fd, recv_message, sizeof(*recv_message), 0, (struct sockaddr *) address, socket_len);
	if(recv_msg_size < 0 && errno !=EAGAIN){
		perror("Error receiving message: ");
	}
	return recv_msg_size;
}

// Function for handling zombie process
void zombie_handler_func(int sigtemp_fd)
{
    wait(NULL);
}

int get_mode (char *mode_checker)
{
    int transfermode = 0;

    if (strcasecmp(mode_checker, "netascii") == 0)
        transfermode = NETASCII;

    if (strcasecmp(mode_checker, "octet") == 0)
        transfermode = OCTET;

    return transfermode;
}

void handle_message(tftp_message_t *message, ssize_t msglen, struct sockaddr_in *client_socket, socklen_t socket_length)
{
    tftp_message_t client_message;
    int            socket_fd;
    ssize_t        data_length, send_receive_size;
    uint16_t       block_number = 0;
    struct timeval tv;
    char          *file_name, *mode_checker, *end;
    FILE          *fd, *temp_fd;
    int            transfermode = 0;
    uint16_t       OpCode;
    uint8_t        data[512];
    int            retry_count;
    int            exit_flag = 0;
    char           temp_file_name[50];


    // opening new socket to handle incoming request

    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("socket() fail");
        exit(-1);
    }

    tv.tv_usec = 0;
    tv.tv_sec = TIMEOUT;


    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        perror("setsockopt() fail");
        exit(-1);
    }

    printf("Got a new connection from %s':%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
    file_name = (char *)message->tftp_request_message.uiFileNameMode;
    end = &file_name[msglen - 3];

    if (*end != '\0')
    {
        printf("Invalid filename or mode requested by %s':%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
        send_error_message(socket_fd, 0, "Invalid filename or mode", client_socket, socket_length);
        exit(-1);
    }

    mode_checker = strchr(file_name, '\0') + 1;

    if (mode_checker > end)
    {
        printf("transfer mode not not given by %s:%u\n",inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
        send_error_message(socket_fd, 0, "transfer mode not given", client_socket, socket_length);
        exit(-1);
    }

    OpCode = ntohs(message->uiOpcode);
    fd = fopen(file_name, OpCode == RRQ_OPCODE ? "r" : "w");
    if (fd == NULL)
    {
        printf("File not found\n");
        send_error_message(socket_fd, 1, "File not found", client_socket, socket_length);
        exit(-1);
    }

   transfermode = get_mode(mode_checker);
   if (transfermode == 0)
   {
       printf("Invalid transfer mode set by %s:%u\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
       send_error_message(socket_fd, 0, "Invalid mode", client_socket, socket_length);
       exit(-1);
   }

   printf("File %s in mode %s will be %s by %s:%u! \n", file_name, mode_checker,
            ntohs(message->uiOpcode) == RRQ_OPCODE ? "read" : "written", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));

    if (OpCode == RRQ_OPCODE)
    {
        if (transfermode == NETASCII)
        {
            temp_fd = create_temp_file(fd,temp_file_name);
            fclose (fd);
            fd = temp_fd;
        }
        while (!exit_flag)
        {
            data_length = fread(data, 1, sizeof(data), fd);
            block_number++;

            if (data_length < 512)
            {
                exit_flag = 1;		// terminating data block
            }

            retry_count = MAX_RETRIES;
            while (retry_count)
            {
                if (send_data(socket_fd, transfermode, block_number, data, data_length, client_socket, socket_length) < 0)
                {
                    printf("session terminated for %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                    exit(-1);
                }

                send_receive_size = receive_message(socket_fd, &client_message, client_socket, &socket_length);

                if (send_receive_size >= 0 && send_receive_size < 4)
                {
                    printf("Invalid req size received from %s:%u\n",inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                    send_error_message(socket_fd, 0, "Invalid req size", client_socket, socket_length);
                    exit(-1);
                }

                if (send_receive_size >= 4)
                {
                    break;
                }

                if (errno != EAGAIN)
                {
                    printf("session terminated for %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                    exit(-1);
                }

                printf("no response session from %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                retry_count--;
            }

            if (retry_count == 0)
            {
                printf("Transfer session has failed\n");
                exit(-1);
            }

            if (ntohs(client_message.uiOpcode) == ERROR_OPCODE)
            {
                printf("Error message %u:%s received for %s:%u\n", ntohs(client_message.tftp_error_message.uiErrorCode),
                        client_message.tftp_error_message.uiErrorData, inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                exit(-1);
            }

            if (ntohs(client_message.uiOpcode) != ACK_OPCODE)
            {
                printf("unknown message received from %s:%u!\n",	inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                send_error_message(socket_fd, 0, "unexpected message received", client_socket, socket_length);
                exit(-1);
            }

            if (ntohs(client_message.tftp_ack_message.uiBlockNumber) != block_number) // size of Ack Block Number is too high
            {
                printf("Invalid acknowledgment received from %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                send_error_message(socket_fd, 0, "Invalid acknowledgment number", client_socket, socket_length);
                exit(-1);
            }
        }
    }

    else if (OpCode == WRQ_OPCODE)
    {
        exit_flag = 0;
        send_receive_size = 0;
        block_number = 0;
        tftp_message_t client_message;

        if (send_ack(socket_fd, block_number, client_socket, socket_length) < 0)
        {
            printf("session terminated for %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
            exit(-1);
        }

        while (!exit_flag)
        {
            retry_count = MAX_RETRIES;
            while(retry_count)
            {
                send_receive_size = receive_message(socket_fd, &client_message, client_socket, &socket_length);

                if (send_receive_size >= 0 && send_receive_size < 4)
                {
                    printf("Invalid req size received from %s:%u\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                    send_error_message(socket_fd, 0, "Invalid req size", client_socket, socket_length);
                    exit(-1);
                }

                if (send_receive_size >= 4)
                {
                    break;
                }

                if (errno != EAGAIN)
                {
                    printf("session terminated for %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                    exit(-1);
                }

                send_receive_size = send_ack(socket_fd, block_number, client_socket, socket_length);

                if (send_receive_size < 0)
                {
                    printf("session terminated for %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                    exit(-1);
                }
                retry_count--;
            }

            if (retry_count == 0)
            {
                printf("no response from %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                exit(-1);
            }

            block_number++;

            if (send_receive_size < sizeof(client_message.tftp_data_message))				//Last block in transfer session
            {
                exit_flag = 1;
            }

            if (ntohs(client_message.tftp_ack_message.uiBlockNumber) != block_number)
            {
                printf("Invalid Block received from %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                send_error_message(socket_fd, 0, "Invalid Block number", client_socket, socket_length);
                exit(-1);
            }

            if (ntohs(client_message.uiOpcode) != DATA_OPCODE)
            {
                printf("unexpected data received from %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                send_error_message(socket_fd, 0, "unexpected data received", client_socket, socket_length);
                exit(-1);
            }

            if (ntohs(client_message.uiOpcode) == ERROR_OPCODE)
            {
                printf("Error message %u:%s received for %s:%u\n", ntohs(client_message.tftp_error_message.uiErrorCode),
                        client_message.tftp_error_message.uiErrorData, inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                exit(-1);
            }


            send_receive_size = fwrite(client_message.tftp_data_message.data, 1, send_receive_size - 4, fd);

            if (send_receive_size < 0)
            {
                perror("write()");
                exit(-1);
            }

            send_receive_size = send_ack(socket_fd, block_number, client_socket, socket_length);

            if (send_receive_size < 0)
            {
                printf("session terminated for %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
                exit(-1);
            }

        }
    }

    printf("Data transfer done successfully for %s:%u!\n", inet_ntoa(client_socket->sin_addr), ntohs(client_socket->sin_port));
    fclose(fd);
    if(temp_file_name != NULL){
        remove(temp_file_name);
    }
    close(socket_fd);
    exit(0);
}

