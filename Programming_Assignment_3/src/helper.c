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

void update_message(char * message, int index, int new_line){
    int i;
	for(i = FILE_SIZE-1; i > index; i--){
		message[i] = message[i-1];
	}
	if(new_line == 0){
		message[index+1] = '\0';
	}
	else
	{
		message[index] = '\r';
		message[index+1] = '\n';
	}
}

ssize_t send_data(int write_fd, int mode, uint16_t uiBlockNumber, uint8_t *data, ssize_t message_length, struct sockaddr_in * address, socklen_t socket_len){
	char msg_body[FILE_SIZE];
	tftp_message_t data_message;
	int extra_char = 0;
	int total_len = 0;
    int index;
	memcpy(msg_body, data, message_length);
	if(mode == NETASCII){
		for(index=0; index <=FILE_SIZE-1; index++)
		{
			if(msg_body[index] == '\r'){
				update_message(msg_body, index, 0);
				index += 2;
				extra_char++;
			}
			if(msg_body[index] == '\n'){
				index +=2;
				extra_char++;
			}
		}
	}
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
void zombie_handler_func(int signum)
{
    wait(NULL);
}


































