#include "common.h"
is

int main (int argc,char *argv[])
{
    int     iSocket_fd = -1, iRet = -1, port_number = -1;
    char    acSendData[BUFFER_SIZE];
    char    *pcUserName, *pcIpAddress;
    char    acRecvData[BUFFER_SIZE + 1];
    struct  sockaddr_in sSocket_addr;
    struct  addrinfo hints, *servinfo, *p;
    struct addrinfo *result, *tempresult;
    port_number = atoi(argv[3]);

    if (argc != 4)
    {
        printf ("ERROR: Please provide User name, IP address and port number\n");
        return 0;
    }

    pcIpAddress = argv[2];
    pcUserName = argv[1];
    printf ("CLIENT: Port number is %d \n", port_number);
    printf ("CLIENT: IP Address is %s \n", pcIpAddress);
    printf ("CLIENT: User Name is %s \n", pcUserName);

    bzero (&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if((iRet = getaddrinfo (pcIpAddress, port_number, &hints, &result))!=0)
        printf("getaddrinfo error for %s, %s; %s", pcIpAddress, port_number, gai_strerror(iRet));

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

#if 0
    iSocket_fd = socket (AF_INET, SOCK_STREAM, 0);
    if (iSocket_fd < 0)
        perror ("ERR: Socket Error");

    /* Initialize the socket address */
    memset (&sSocket_addr, 0, sizeof sSocket_addr);
    sSocket_addr.sin_family = AF_INET;
    sSocket_addr.sin_port = htons (port_number);
    iRet = inet_pton (AF_INET, argv[1], &(sSocket_addr.sin_addr));
    if (iRet < 0)
        perror ("ERROR: IP conversion failed");

    /* Connect to server */
    iRet = connect (iSocket_fd, (struct sockaddr *)&sSocket_addr, sizeof(sSocket_addr));
    if (iRet < 0)
        perror ("ERROR: Socket connection failed");

    printf("Connection is Established with the Server...\n");
    while(1)
    {
        memset (acSendData, 0, BUFFER_SIZE);
        memset (acRecvData, 0, BUFFER_SIZE + 1);
        /* Get data from stdin */
        fgets (acSendData, BUFFER_SIZE, stdin);
        if (strlen (acSendData) == 0)
        {
            printf("Connection closed with the Server because EOF...\n");
            close(iSocket_fd);
            exit(0);
        }
        printf ("CLIENT Sending data to server: %s\n", acSendData);
        iRet = writen (iSocket_fd, acSendData, strlen (acSendData));
        if (iRet < 0)
        {
            perror ("ERROR: Writen failed to send data");
            break;
        }
        iRet = iReadLine (iSocket_fd, acRecvData, BUFFER_SIZE);
        if (iRet < 0)
        {
            perror ("ERROR: Readline failed to receive data");
            break;
        }
        else
        {
            acRecvData [iRet] = '\0';
        }
        iRet = 0;
        printf ("CLIENT Receiving data from server: %s \n", acRecvData);
    }
    return 0;
#endif

}
