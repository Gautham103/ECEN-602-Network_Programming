#include "common.h"

char *pcDay[7]=
{
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"
};

char *pcMonth[12]=
{
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
};

int iHandle_Client_Message (int client_fd)
{
    struct addrinfo hints, *result;
    int iPort = HTTP_PORT_NUMBER;
    int porxy_fd;
    char acBuffer[BUFFER_SIZE], acHost[MAX_NAME_LENGTH], acUrl[MAX_NAME_LENGTH], acName[MAX_NAME_LENGTH];
    time_t now;
    struct tm *timenow;
    int recvlen = -1;
    struct sockaddr_in proxyaddr;
    char acRequest[1024];
    int  iSendLen;
    int i = 0,iOldest_Entry = 0;
    FILE *fp;
    FILE *readfp;
    char* expires=NULL;
    int iCache_Num;
    int readlen = 0;
    char modified[100];
    char modified_request[BUFFER_SIZE];

    memset(acBuffer, 0, BUFFER_SIZE);
    memset(acHost, 0, MAX_NAME_LENGTH);
    memset(acUrl, 0, MAX_NAME_LENGTH);
    memset(acName, 0, MAX_NAME_LENGTH);
    if(recv(client_fd, acBuffer, sizeof(acBuffer), 0) < 0)
    {
        perror("recv");
        close(client_fd);
        return 1;
    }

    if (iFormat_Read_Request(acBuffer, acHost, &iPort, acUrl, acName) != 4)
    {
        vSend_Error_Message(400,client_fd);
        close(client_fd);
        return 1;
    }

    printf("Request\nMethod:%s\nHost:%s\nFile:%s\n","GET",acHost,acName);


    memset(&proxyaddr, 0, sizeof(proxyaddr));
    proxyaddr.sin_addr.s_addr = INADDR_ANY;
    proxyaddr.sin_family = AF_INET;

    if((porxy_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("proxysocket create");
        close(client_fd);
        return 1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if(getaddrinfo(acHost, "80", &hints, &result)!=0)
    {
        printf("getaddrinfo error\n");
        vSend_Error_Message(404, client_fd);
        close(client_fd);
        return 1;
    }

    if (connect(porxy_fd, result->ai_addr, result->ai_addrlen) < 0)
    {
        close(porxy_fd);
        perror("connect error:");
        vSend_Error_Message(404, client_fd);
        close(client_fd);
        return 1;
    }

    time(&now);
    timenow = gmtime(&now);
    if(iCheck_Cache_Entry_Hit(acUrl) == -1)
    {
        memset(acRequest, 0, 1024);
        printf("Cannnot find requested file in cache..\ndownloading from %s:%d\n",acHost,iPort);
        sprintf(acRequest, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", acName,acHost);
        puts(acRequest);
        if((iSendLen = send(porxy_fd, acRequest, strlen(acRequest), 0)) < 0)
        {
            perror("send acRequest:");
            close(porxy_fd);
            return 1;
        }
        printf("Request sent to server.\n");
        memset(acBuffer, 0, BUFFER_SIZE);
        for (i = 0; i < 10; i++)
        {
            if(sCache_table[i].iIs_filled == 0)
            {
                iOldest_Entry = i;
                break;
            }
            else
            {
                if (iTime_Comparison_Func(sCache_table[i].acLast_Modified_Time,sCache_table[iOldest_Entry].acLast_Modified_Time) <= 0)
                {
                    iOldest_Entry = i;
                }
            }
        }

        if (pthread_mutex_lock(&mFile_Lock[iOldest_Entry]) == 0)
        {
            memset(&sCache_table[iOldest_Entry], 0, sizeof(sCache));
            sCache_table[iOldest_Entry].iIs_filled = 1;
            sprintf(sCache_table[iOldest_Entry].acFilename, "%d", iOldest_Entry);
            memcpy(sCache_table[iOldest_Entry].acUrl, acUrl, MAX_NAME_LENGTH);
            sprintf(sCache_table[iOldest_Entry].acLast_Modified_Time, "%s, %02d %s %d %02d:%02d:%02d GMT", pcDay[timenow->tm_wday],timenow->tm_mday, pcMonth[timenow->tm_mon], timenow->tm_year+1900,timenow->tm_hour,timenow->tm_min,timenow->tm_sec);

            remove(sCache_table[iOldest_Entry].acFilename);
            fp=fopen(sCache_table[iOldest_Entry].acFilename, "w");
            if (fp==NULL)
            {
                printf("failed to create cache.\n");
                pthread_mutex_unlock(&mFile_Lock[iOldest_Entry]);
                return 1;
            }
            printf("Last-Modified:");
            puts(sCache_table[iOldest_Entry].acLast_Modified_Time);
            printf("\n");
            while ((recvlen=recv(porxy_fd, acBuffer, BUFFER_SIZE, 0)) > 0)
            {
                if(send(client_fd, acBuffer, recvlen, 0) < 0)
                {
                    perror("Client send:");
                    pthread_mutex_unlock(&mFile_Lock[iOldest_Entry]);
                    return 1;
                }
                fwrite(acBuffer, 1, recvlen, fp);
                memset(acBuffer, 0, BUFFER_SIZE);
            }
            printf("Received successfull response from server.\n");
            printf("Transferring file to client successful.\n*************************\n");
            fclose(fp);
            readfp=fopen(sCache_table[iOldest_Entry].acFilename,"r");
            fread(acBuffer, 1, 2048, readfp);
            fclose(readfp);

            expires=strstr(acBuffer, "Expires: ");
            if (expires!=NULL)
            {
                memcpy(sCache_table[iOldest_Entry].acExpiry, expires + 9, 29);
            }
            else
            {
                sprintf(sCache_table[iOldest_Entry].acExpiry, "%s, %02d %s %4d %02d:%02d:%02d GMT", pcDay[timenow->tm_wday],timenow->tm_mday, pcMonth[timenow->tm_mon], timenow->tm_year+1900,(timenow->tm_hour),(timenow->tm_min)+2,timenow->tm_sec);
            }
            pthread_mutex_unlock(&mFile_Lock[iOldest_Entry]);
        }
        else
        {
            printf("Another thread is accessing same cache file\n");
            while ((recvlen=recv(porxy_fd, acBuffer, BUFFER_SIZE, 0))>0)
            {

                if(send(client_fd, acBuffer, recvlen, 0) == -1)
                {
                    perror("Send to client:");
                    return 1;
                }
                memset(acBuffer, 0, BUFFER_SIZE);
            }
        }

    }
    else
    {
        if(iCheck_Cache_Entry_Expire(acUrl,timenow,&iCache_Num)>=0)
        {
            printf("file found in cache and its not expired.\n sending to client...\n");
            sprintf(sCache_table[iCache_Num].acLast_Modified_Time, "%s, %02d %s %4d %02d:%02d:%02d GMT", pcDay[timenow->tm_wday],timenow->tm_mday, pcMonth[timenow->tm_mon], timenow->tm_year+1900,timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
            readfp=fopen(sCache_table[iCache_Num].acFilename,"r");
            memset(acBuffer, 0, BUFFER_SIZE);
            while ((readlen=fread(acBuffer, 1, BUFFER_SIZE, readfp)) > 0)
            {
                send(client_fd, acBuffer, readlen, 0);
            }
            printf("Transferring file to client successfully sent to client.\n**********************\n");
            fclose(readfp);
        }
        else
        {
            printf("file has expired, fetching updated file from server.\n");
            memset(modified, 0, 100);
            sprintf(modified, "If-Modified-Since: %s\r\n\r\n",sCache_table[iCache_Num].acLast_Modified_Time);
            memset(modified_request, 0, BUFFER_SIZE);
            memcpy(modified_request, acBuffer, strlen(acBuffer)-2);

            strcat(modified_request, modified);
            printf("%s\n",modified_request);
            send(porxy_fd, modified_request, strlen(modified_request), 0);
            memset(acBuffer, 0, BUFFER_SIZE);
            recvlen = recv(porxy_fd, acBuffer, BUFFER_SIZE, 0);
            expires=strstr(acBuffer, "Expires: ");
            if (expires!=NULL)
            {
                memcpy(sCache_table[iCache_Num].acExpiry, expires+9, 29);
            }
            else
            {
                sprintf(sCache_table[iCache_Num].acExpiry, "%s, %02d %s %4d %02d:%02d:%02d GMT", pcDay[timenow->tm_wday],timenow->tm_mday, pcMonth[timenow->tm_mon], timenow->tm_year+1900,timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
            }

            if (recvlen > 0)
            {
                if ((*(acBuffer + 9) == '3') && (*(acBuffer + 10) == '0') && (*(acBuffer + 11) == '4'))
                {
                    printf("%s",acBuffer);
                    printf("File is still up to date. Sending file in cache.\n*********************************************************\n");

                    readfp = fopen(sCache_table[iCache_Num].acFilename,"r");
                    memset(acBuffer, 0, BUFFER_SIZE);
                    while ((readlen=fread(acBuffer, 1, BUFFER_SIZE, readfp))>0)
                    {
                        send(client_fd, acBuffer, readlen, 0);
                    }
                    fclose(readfp);

                }
                else if((*(acBuffer + 9) == '4') && (*(acBuffer + 10) == '0') && (*(acBuffer + 11) == '4'))
                {
                    vSend_Error_Message( 404, client_fd);
                }
                else if((*(acBuffer + 9) == '2') && (*(acBuffer + 10) == '0') && (*(acBuffer + 11) == '0'))
                {
                    printf("New file received from server.\nUpdating cache and sending file to client.\n*********************************************************\n");
                    send(client_fd, acBuffer, recvlen, 0);
                    if (pthread_mutex_lock(&mFile_Lock[iCache_Num])==0)
                    {
                        remove(sCache_table[iCache_Num].acFilename);

                        expires = NULL;

                        expires = strstr(acBuffer, "Expires: ");
                        if (expires != NULL)
                        {
                            memcpy(sCache_table[iCache_Num].acExpiry, expires+9, 29);
                        }
                        else
                        {
                            sprintf(sCache_table[iCache_Num].acExpiry, "%s, %02d %s %4d %02d:%02d:%02d GMT", pcDay[timenow->tm_wday],timenow->tm_mday, pcMonth[timenow->tm_mon], timenow->tm_year+1900,timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
                        }

                        sprintf(sCache_table[iCache_Num].acLast_Modified_Time, "%s, %02d %s %4d %02d:%02d:%02d GMT", pcDay[timenow->tm_wday],timenow->tm_mday, pcMonth[timenow->tm_mon], timenow->tm_year+1900,timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
                        fp=fopen(sCache_table[iCache_Num].acFilename, "w");
                        fwrite(acBuffer, 1, recvlen, fp);

                        memset(acBuffer, 0, BUFFER_SIZE);
                        while ((recvlen = recv(porxy_fd, acBuffer, BUFFER_SIZE, 0)) > 0)
                        {
                            send(client_fd, acBuffer, recvlen, 0);
                            fwrite(acBuffer, 1, recvlen, fp);
                        }
                        fclose(fp);
                        pthread_mutex_unlock(&mFile_Lock[iCache_Num]);
                    }
                    else
                    {
                        printf("Another thread is trying to operate on the same cache file.\n");
                        memset(acBuffer, 0, BUFFER_SIZE);
                        while ((recvlen = recv(porxy_fd, acBuffer, BUFFER_SIZE, 0)) > 0)
                        {
                            send(client_fd, acBuffer, recvlen, 0);
                        }
                    }
                }
            }
            else
                perror("receive:");
        }

    }
    close(client_fd);
    close(porxy_fd);
    return 0;
}

int main(int argc, char *argv[])
{
    pthread_t thread_id;
    int iPort,iListener_fd,client_fd;
    struct sockaddr_storage remoteaddr;
    struct sockaddr_in addr;
    int j;
    socklen_t sin_size;
    if (argc<3)
    {
        printf("\nUsage: server host port\nExiting\n");
        exit(-1);
    }
    memset(&remoteaddr, 0, sizeof remoteaddr);
    memset(&addr, 0, sizeof addr);

    iPort = atoi(argv[2]);
    addr.sin_port = htons(iPort);
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_family = AF_INET;

    if((iListener_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Cannot create socket");
        exit(-1);
    }
    if(bind(iListener_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr)) < 0)
    {
        perror("Cannot bind socket");
        exit(-1);
    }
    if(listen(iListener_fd, 10) < 0)
    {
        perror("listen");
        exit(-1);
    }

    for ( j=0; j<10; j++)
    {
        pthread_mutex_init(&mFile_Lock[j], NULL);
    }

    memset(sCache_table, 0, 10 * sizeof(sCache));

    printf("Waiting for request!\n");
    sin_size = sizeof(remoteaddr);
    while (1)
    {
        client_fd = accept(iListener_fd, (struct sockaddr *)&remoteaddr, &sin_size);
        if (iListener_fd == -1)
        {
            perror("accept:");
            continue;
        }
        pthread_create(&thread_id, NULL, (void *)(&iHandle_Client_Message), (void *)(intptr_t) client_fd);
    }

    return 0;
}
