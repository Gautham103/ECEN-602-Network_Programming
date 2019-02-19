#include "common.h"

int main (int argc,char *argv[])
{
    int              iSocket_fd = -1, iRet = -1, port_number = -1;
    char             acChatData[PAYLOAD_SIZE];
    char            *pcUserName, *pcIpAddress;
    struct  addrinfo hints ,*result, *tempresult;
    fd_set           fdRead_Set;
    int              fdMax;
    sbcp_message_t  *psMessage = NULL;
    unsigned int     uiChatLength;

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

    bzero (&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if((iRet = getaddrinfo (pcIpAddress, argv[3], &hints, &result))!=0)
        printf("getaddrinfo error for %s, %d; %s", pcIpAddress, port_number, gai_strerror(iRet));

    tempresult = result;

    do
    {
        iSocket_fd = socket (result->ai_family, result->ai_socktype, result->ai_protocol);

        if (iSocket_fd < 0)
            continue;  /*ignore this returned Ip addr*/

        if (connect (iSocket_fd, result->ai_addr, result->ai_addrlen) == 0)
        {
            printf("connection ok!\n"); /* success*/
            break;
        }
        else
        {
            perror("connection NOT ok");
        }


        close (iSocket_fd);/*ignore this one*/
    } while ((result=result->ai_next)!= NULL);

    freeaddrinfo(tempresult);

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
    while(1)
    {
        fdMax = iSocket_fd;
        if (select (fdMax + 1, &fdRead_Set, NULL, NULL, NULL) == -1)
        {
            perror("ERR: Select Error");
            exit(6);
        }

        if (FD_ISSET(STDIN_FD, &fdRead_Set))
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

            if(psMessage->sMsgHeader.uiType == FWD && psMessage->sMsgAttribute.uiType == MESSAGE)
            {
                if((psMessage->sMsgAttribute.acPayload != NULL || psMessage->sMsgAttribute.acPayload !='\0'))
                {
                    printf("%s", psMessage->sMsgAttribute.acPayload);
                }
            }

            if(psMessage->sMsgHeader.uiType == NAK && psMessage->sMsgAttribute.uiType == REASON)
            {
                if((psMessage->sMsgAttribute.acPayload != NULL || psMessage->sMsgAttribute.acPayload !='\0'))
                {
                    printf("NAK received from the server: %s",psMessage->sMsgAttribute.acPayload);
                    if (psMessage != NULL)
                        free (psMessage);
                    exit (2);
                }
            }
            if (psMessage != NULL)
                free (psMessage);

        }

    }
}
