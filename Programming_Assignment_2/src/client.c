#include "common.h"
pthread_mutex_t ready_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ready_cond = PTHREAD_COND_INITIALIZER;
bool isIdleMessageSent = false;

int main (int argc,char *argv[])
{
    int              iSocket_fd = -1, iRet = -1, port_number = -1;
    char             acChatData[PAYLOAD_SIZE];
    char            *pcUserName, *pcIpAddress;
    fd_set           fdRead_Set;
    int              fdMax;
    sbcp_message_t  *psMessage = NULL;
    unsigned int     uiChatLength;
    pthread_attr_t   sThreadAttr;
    pthread_t        iThreadId = 0;
    struct sockaddr_in client_address_ipv4;
    struct sockaddr_in6 client_address_ipv6;
    struct addrinfo hint, *res = NULL;
    int ret;

    port_number = atoi(argv[3]);

    if (argc != 4)
    {
        printf ("ERROR: Please provide User name, IP address and port number\n");
        return 0;
    }

    pcIpAddress = argv[2];
    pcUserName = argv[1];
    printf ("CLIENT: User Name is %s \n", pcUserName);
    printf ("CLIENT: IP Address is %s \n", pcIpAddress);
    printf ("CLIENT: Port number is %d \n", port_number);

    memset(&hint, '\0', sizeof hint);

    hint.ai_family = PF_UNSPEC;
    hint.ai_flags = AI_NUMERICHOST;

    // Figure out whether the address is IPv4 or IPv6 and the call the APIs accordingly
    ret = getaddrinfo(pcIpAddress, NULL, &hint, &res);
    if (ret)
    {
        puts("Invalid address");
        puts(gai_strerror(ret));
        return 1;
    }
    if(res->ai_family == AF_INET)
    {
        iSocket_fd = create_socket(true);
        memset (&client_address_ipv4, 0, sizeof client_address_ipv4);
        client_address_ipv4.sin_family = AF_INET;
        client_address_ipv4.sin_port = htons (port_number);
        iRet = inet_pton (AF_INET, pcIpAddress, &(client_address_ipv4.sin_addr));
        if (iRet < 0)
            perror ("ERROR: IP conversion failed");

        /* Connect to server */
        iRet = connect (iSocket_fd, (struct sockaddr *)&client_address_ipv4, sizeof(client_address_ipv4));
        if (iRet < 0)
            perror ("ERROR: Socket connection failed");
    }
    else if (res->ai_family == AF_INET6)
    {
        iSocket_fd = create_socket(false);
        memset (&client_address_ipv6, 0, sizeof client_address_ipv4);
        client_address_ipv6.sin6_family = AF_INET6;
        client_address_ipv6.sin6_port = htons (port_number);
        inet_pton(AF_INET6, pcIpAddress, &client_address_ipv6.sin6_addr);

        /* Connect to server */
        iRet = connect (iSocket_fd, (struct sockaddr *)&client_address_ipv6, sizeof(client_address_ipv6));
        if (iRet < 0)
            perror ("ERROR: Socket connection failed");
    }
    else
    {
        printf("%s is an is unknown address format %d\n",argv[1],res->ai_family);
    }

   freeaddrinfo(res);

    psMessage = (sbcp_message_t *)malloc (sizeof (sbcp_message_t));
    bzero (psMessage, sizeof (sbcp_message_t));

    // SBCP Header - JOIN
    psMessage->sMsgHeader.uiVrsn = PROTOCOL_VERSION;
    psMessage->sMsgHeader.uiType = JOIN;
    psMessage->sMsgHeader.uiLength = 20;

    // SBCP Attribute - USERNAME
    psMessage->sMsgAttribute.uiType = USERNAME;
    psMessage->sMsgAttribute.uiLength = (4 + strlen(pcUserName));
    bzero(psMessage->sMsgAttribute.acPayload, sizeof(psMessage->sMsgAttribute.acPayload));
    strcpy (psMessage->sMsgAttribute.acPayload, pcUserName);

    // Sending JOIN message to server
    iRet = send (iSocket_fd, (sbcp_message_t *)psMessage, sizeof(sbcp_message_t), 0);
    if (iRet < 0)
    {
        perror ("ERROR: Writen failed to send data");
    }
    printf("The join message has been sent successfully\n");
    if (psMessage != NULL)
    free (psMessage);

    printf("Connection is Established with the Server...\n");
    FD_SET(0, &fdRead_Set);              // add user input from keyboard to the Read_FDs set
    FD_SET(iSocket_fd, &fdRead_Set); // add sockfd to the Read_FDs set

    pthread_attr_init (&sThreadAttr);
    pthread_attr_setdetachstate (&sThreadAttr, PTHREAD_CREATE_DETACHED);

    iRet = pthread_create ((pthread_t *)&iThreadId, &sThreadAttr,
            (void *)&vSendIdleMessage,
            (void *)&iSocket_fd);


    while(1)
    {
        fdMax = iSocket_fd;
        if (select (fdMax + 1, &fdRead_Set, NULL, NULL, NULL) == -1)
        {
            perror("ERR: Select Error");
            exit(6);
        }

        if (FD_ISSET(0, &fdRead_Set))
        {
                bzero(acChatData, PAYLOAD_SIZE);
                fgets(acChatData, PAYLOAD_SIZE, stdin);
                uiChatLength = strlen(acChatData) - 1;
                if(acChatData[uiChatLength] == '\n')
                {
                    acChatData[uiChatLength] = '\0';                // Provision to detect unceremonious exit from client
                }

                psMessage = (sbcp_message_t *)malloc (sizeof (sbcp_message_t));
                bzero (psMessage, sizeof (sbcp_message_t));

                // SBCP Header - SEND
                psMessage->sMsgHeader.uiVrsn = PROTOCOL_VERSION;
                psMessage->sMsgHeader.uiType = SEND;
                psMessage->sMsgHeader.uiLength = 520;

                // SBCP Header - MESSAGE
                psMessage->sMsgAttribute.uiType = MESSAGE;
                psMessage->sMsgAttribute.uiLength = 524;
                bzero (psMessage->sMsgAttribute.acPayload, sizeof(psMessage->sMsgAttribute.acPayload));
                strcpy (psMessage->sMsgAttribute.acPayload, acChatData);

                pthread_mutex_lock(&ready_mutex);
                pthread_cond_signal(&ready_cond);
                isIdleMessageSent = false;
                pthread_mutex_unlock(&ready_mutex);
                // Sending SEND message to server
                iRet = send(iSocket_fd, (sbcp_message_t *)psMessage, sizeof(sbcp_message_t), 0);
                if (iRet < 0)
                {
                    perror ("ERROR: Writen failed to send data");
                }
                if (psMessage != NULL)
                    free (psMessage);

        }

        if (FD_ISSET(iSocket_fd, &fdRead_Set))
        {
            psMessage = (sbcp_message_t *)malloc (sizeof (sbcp_message_t));
            bzero (psMessage, sizeof (sbcp_message_t));
            recv(iSocket_fd, (sbcp_message_t *)psMessage, sizeof(sbcp_message_t), 0);

            if((psMessage->sMsgHeader.uiType == FWD || psMessage->sMsgHeader.uiType == ACK)
                    && psMessage->sMsgAttribute.uiType == MESSAGE)
            {
                // FWD/ACK Message received. Display the message
                if((psMessage->sMsgAttribute.acPayload != NULL || psMessage->sMsgAttribute.acPayload !='\0'))
                {
                    printf("%s\n", psMessage->sMsgAttribute.acPayload);
                }
            }

            if(psMessage->sMsgHeader.uiType == NAK && psMessage->sMsgAttribute.uiType == REASON)
            {
                // NAK Message received. Display the reason
                if((psMessage->sMsgAttribute.acPayload != NULL || psMessage->sMsgAttribute.acPayload !='\0'))
                {
                    printf("NAK received from the server: %s\n",psMessage->sMsgAttribute.acPayload);
                    if (psMessage != NULL)
                        free (psMessage);
                    exit (2);
                }
            }
            if (psMessage != NULL)
                free (psMessage);

        }
        FD_SET(0, &fdRead_Set);
        FD_SET(iSocket_fd, &fdRead_Set);

    }
}

void vSendIdleMessage (void *arg)
{
    int *iSocket_fd = (int *)arg;
    int iRet = 0;
    time_t T;
    struct timespec t;
    sbcp_message_t  *psMessage = NULL;

    while (1)
    {
WaitAgain:
        time(&T);
        t.tv_sec = T + IDLE_WAIT_TIME;
        pthread_mutex_lock(&ready_mutex);
        iRet = pthread_cond_timedwait(&ready_cond, &ready_mutex, &t);
        pthread_mutex_unlock(&ready_mutex);
        if (iRet != 0)
        {
            if (errno == EAGAIN)
            {

            }
            else
            {
                if (isIdleMessageSent == false)
                {
                    isIdleMessageSent = true;
                    psMessage = (sbcp_message_t *)malloc (sizeof (sbcp_message_t));
                    bzero (psMessage, sizeof (sbcp_message_t));

                    // SBCP Header - SEND
                    psMessage->sMsgHeader.uiVrsn = PROTOCOL_VERSION;
                    psMessage->sMsgHeader.uiType = IDLE;
                    psMessage->sMsgHeader.uiLength = 520;

                    // Sending SEND message to server
                    iRet = send(*iSocket_fd, (sbcp_message_t *)psMessage, sizeof(sbcp_message_t), 0);
                    if (iRet < 0)
                    {
                        perror ("ERROR: Writen failed to send data");
                    }
                    if (psMessage != NULL)
                        free (psMessage);
                }
            }
        }
        else
        {
            goto WaitAgain;
        }

    }

}
