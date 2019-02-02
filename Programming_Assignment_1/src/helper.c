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
        exit(0);
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
        exit(0);
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
        exit(0);
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


