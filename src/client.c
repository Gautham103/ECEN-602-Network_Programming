#include "common.h"

int main (int argc,char *argv[])
{
  int     iSocket_fd = -1, iRet = -1, port_number = -1;
  char    acSendData[QUEUE_SIZE];
  char    acRecvData[QUEUE_SIZE + 1];
  char   *temp_ptr = NULL;
  struct  sockaddr_in sSocket_addr;
  printf ("CLIENT: Port number is %s \n", argv[1]);
  printf ("CLIENT: IP Address is %s \n", argv[2]);
  port_number = atoi(argv[1]);

  if (argc != 3)
  {
    perror ("ERROR: Please provide IP address and port number");
    return 0;
  }

  iSocket_fd = socket (AF_INET, SOCK_STREAM, 0);
  if (iSocket_fd < 0)
    perror ("ERR: Socket Error");

  /* Initialize the socket address */
  memset (&sSocket_addr, 0, sizeof sSocket_addr);
  sSocket_addr.sin_family = AF_INET;
  sSocket_addr.sin_port = htons (port_number);
  iRet = inet_pton (AF_INET, argv[2], &(sSocket_addr.sin_addr));
  if (iRet <= 0)
    perror ("ERROR: IP conversion failed");

  /* Connect to server */
  iRet = connect (iSocket_fd, (struct sockaddr *)&sSocket_addr, sizeof(sSocket_addr));
  if (iRet < 0)
    perror ("ERROR: Socket connection failed");

  while(1)
  {
    memset (acSendData, 0, QUEUE_SIZE);
    memset (acRecvData, 0, QUEUE_SIZE + 1);
    /* Get data from stdin */
    fgets (acSendData, QUEUE_SIZE, stdin);
    writen (iSocket_fd, acSendData, strlen (acSendData));
    if (iRet < 0)
    {
        perror ("ERROR: Writen failed to send data");
        break;
    }
    printf ("CLIENT: Sending data to server \n");
    iRet = iReadLine (iSocket_fd, acRecvData, QUEUE_SIZE);
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
    printf ("CLIENT: Reading data from server %s \n", acRecvData);
  }
return 0;

}
