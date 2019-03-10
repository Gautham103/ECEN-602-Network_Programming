#include "common.h"

int main(int argc, char * argv[]){
	if(argc < 2) {
	        printf("Input Error: Please provide port address as commoand line argument\n");
	        exit(1);
	    }

    // Variable decleration
    char * ip = argv[1];
    int port = atoi(argv[2]);
    int socket_fd = -1;
    struct sockaddr_in server_address, client_address;
    int client_addr_len, pid, connect_fd;
    struct sockaddr_in6 servaddr;
    int socket_itr;

    struct addrinfo hint, *res = NULL;
    int ret;

    memset(&hint, '\0', sizeof hint);

    hint.ai_family = PF_UNSPEC;
    hint.ai_flags = AI_NUMERICHOST;

    // Figure out whether the address is IPv4 or IPv6 and the call the APIs accordingly
    ret = getaddrinfo(argv[1], NULL, &hint, &res);
    if (ret)
    {
        puts("Invalid address");
        puts(gai_strerror(ret));
        return 1;
    }
    if(res->ai_family == AF_INET)
    {
        socket_fd = create_socket(true);
        set_server_address(&server_address, ip, port);
        bind_server(socket_fd, server_address);
    }
    else if (res->ai_family == AF_INET6)
    {
        socket_fd = create_socket(false);
        set_server_address_ipv6(&servaddr, ip, port);
        bind_server_ipv6(socket_fd, servaddr);
    }
    else
    {
        printf("%s is an is unknown address format %d\n",argv[1],res->ai_family);
    }

   freeaddrinfo(res);

    // Initialization
    start_listening(socket_fd);

    client_addr_len = sizeof(client_address);


        while(1)
        {
            connect_fd = accept(socket_fd, (struct sockaddr *) &client_address, (socklen_t *)&client_addr_len);    // Create client socket file descriptor

            if(connect_fd < 0)
            {
                perror("Error Accepting Connection");
            }

            pid = fork();    // Fork a new child process
            if(pid < 0)
            {
                perror("Error Creating Connection");
            }

            if(pid == 0)                                                  // Child process
            {
                close(socket_fd);                                         // child closes listening port
                printf("Connection is Established with the Client...\n");
                printf("Connection closed with the Client...\n");
                close(connect_fd);                                         // Closes the connection with client
                exit(0);
            }
            else                                                           // Parent Process
            {
                signal(SIGCHLD, zombie_handler_func);   // To avoid zombie process
                close(connect_fd);
            }


        }

        close(socket_fd);
}
