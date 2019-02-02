#include "common.h"

int main (int argc,char *argv[])
{
  int     iSocket_fd = -1, iRet = -1, port_number = -1;
  char    acSendData[BUFFER_SIZE];
  char    acRecvData[BUFFER_SIZE + 1];
  struct  sockaddr_in sSocket_addr;
  port_number = atoi(argv[2]);

  if (argc != 3)
  {
    printf ("ERROR: Please provide IP address and port number\n");
    return 0;
  }

  printf ("CLIENT: Port number is %s \n", argv[2]);
  printf ("CLIENT: IP Address is %s \n", argv[1]);
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

}
