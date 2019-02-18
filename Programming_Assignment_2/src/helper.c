#include "common.h"

int writen (int iSocket_fd, char *pcBuffer, int iBufferSize)
{
    int iBytesLeft = iBufferSize, iBytesSent = 0;

    while (iBytesLeft > 0)
    {
        iBytesSent = write (iSocket_fd, pcBuffer, iBytesLeft);
        if (iBytesSent < 0)
        {
            if (errno == EINTR)
            {
                /* EINTR. Retry */
                continue;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            /* Send again if bytes are left */
            iBytesLeft -= iBytesSent;
            pcBuffer += iBytesSent;

        }
    }

    return iBufferSize;
}


int iReadLine (int iSocket_fd, char *pcBuffer, int iBufferSize)
{
    int iLength = 0, iRet = 0;
    char cTempBuffer;

    while (1)
    {
        /* Read one character by character */
        iRet = read (iSocket_fd, &cTempBuffer, 1);
        if (iRet < 0)
        {
            if (errno == EINTR)
            {
                /* EINTR. Retry */
                continue;
            }
            else
            {
                return -1;
            }
        }
        else if (iRet == 0)
        {
            /* No charactered read. EOF */
            *pcBuffer = '\0';
            return iLength;
        }
        else
        {
            if (iLength < iBufferSize - 1)
            {
                *pcBuffer = cTempBuffer;
                pcBuffer++;
                iLength++;
            }

            if (cTempBuffer == '\n')
                break;

            if (iLength == iBufferSize - 1)
                break;

        }

    }
    *pcBuffer = '\0';
    return iLength;

}

int create_socket(){
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd == -1)
    {
        perror("Socket Create Faild");
        exit(-1);
    }
    else{
        printf("Socket is Created Successfully....\n");
    }
    return socket_fd;
}

void set_server_address(struct sockaddr_in *server_address, int port)
{
    bzero(server_address, sizeof(*server_address));
    (*server_address).sin_family = AF_INET;
    (*server_address).sin_addr.s_addr = htonl(INADDR_ANY);
    (*server_address).sin_port = htons(port);
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

void zombie_handler_func(int signum)
{
    wait(NULL);
}

void read_write(int connect_fd)
{
    int read_length = 0;
    while(1)
    {
        char * buffer_array = calloc(BUFFER_SIZE, sizeof(char));
        read_length = iReadLine(connect_fd, buffer_array, BUFFER_SIZE);
        if(read_length == 0)
        {
            free(buffer_array);
            break;
        }
        printf("SERVER DATA READ: %s", buffer_array);
        writen(connect_fd, buffer_array,read_length);
        printf("SENDING DATA BACK TO CLIENT\n");
        free(buffer_array);
    }
}

int accept_connection(struct sockaddr_in * client_addresses, int client_count, int socket_fd)
{
    int len = sizeof(client_addresses[client_count]);
    int new_socket_fd = accept(socket_fd, (struct sockaddr *)&client_addresses[client_count], &len);
    if(new_socket_fd < 0)
    {
        perror("Accept connection error: ");
        exit(-1);
    }
    return new_socket_fd;
}

int check_name(char []name, int client_count, int max_client, struct user_data * clients){
    if(client_count == max_client)
    {
        return -1;
    }
    for(int i=0; i <client_count; i++)
    {
        if(strcmp(name, clients[i].user_name) == 0)
        {
            return -1;
        }
    }
    return 0;
}

void send_ack_message(int new_client_fd, int client_count, struct user_data * clients){
    struct message ack_message;
    sbcp_header_t ack_header;
    sbcp_attribute_t ack_attribute;

    ack_header.uiVrsn = 3;
    ack_header.uiType = 7;

    ack_attribute.uiType = 4;

    char msg[200];
    msg[0] = (char)(((int)'0')+ client_count);
    msg[1] = ' ';
    msg[2] = '\0'
    for(int i=0; i < client_count-1; i++)
    {
        strcat(msg,clients[i].user_name);
        if(i < (client_count-1))
        {
            strcat(msg, ",");
        }
    }
    ack_attribute.uiLength = strlen(msg)+1;
    strcpy(ack_attribute.payload, msg);
    ack_message.sMsgHeader = ack_header;
    ack_message.sMsgAttribute = ack_attribute;

    send(new_client_fd,(void *) &ack_message,sizeof(ack_message),0);
}

void send_nack_message(int new_client_fd, int client_count, struct user_data * clients, int code){
    struct message nack_message;
    sbcp_header_t nack_header;
    sbcp_attribute_t nack_attribute;

    char msg[130];

    nack_header.uiVrsn = 3;
    nack_header.uiType = 5;

    nack_attribute.uiType = 1;

    if(code == 1)
    {
        strcpy(msg,"Ran into some anomaly...please check if username or wait till chatroom is free\n");
    }

    nack_attribute.length = strlen(msg);
    strcpy(nack_attribute.payload, msg);

    nack_message.header = nack_header;
    nack_message.attribute[0] = nack_attribute`;

    send(new_client_fd,(void *) &nack_message,sizeof(nack_message),0);

    close(new_client_fd);

} 

int join_message_process(int new_client_fd, int &client_count, int max_client, struct user_data * clients){
    struct message join_message;
    sbcp_attribute_t join_message_attr;
    recv(new_client_fd,(struct message *) &join_message,sizeof(join_message),0);
    join_message_attr = join_message.sMsgAttribute;
    char name[16];
    strcpy(name, join_message_attr.payload);
    if(check_name(name, *client_count, max_client, clients) == -1)
    {
        printf("User name already exist.\n");
        sendNAK(new_client_fd, client_count, clients,1);
        return 1;
    }
    else
    {
        strcpy(clients[*client_count].user_name, name);
        clients[*client_count].socket_fd = new_client_fd;
        clients[*client_count].user_number = *client_count;
        *client_count = (*client_count) + 1;
    }
    return 0;
}




