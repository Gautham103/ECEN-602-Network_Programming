#include "common.h"

int main(int argc, char * argv[]){
    if(argc < 2) {
        printf("Input Error: Please provide port address as commoand line argument\n");
        exit(1);
    }
    int port = atoi(argv[1]);
    int client_addr_len;
    int socket_fd = create_socket();
    int pid, connect_fd;
    struct sockaddr_in server_address, client_address;

    set_server_address(&server_address, port);
    bind_server(socket_fd, server_address);
    start_listening(socket_fd);
    client_addr_len = sizeof(client_address);

    while(1)
    {
        connect_fd = accept(socket_fd, (struct sockaddr *) &client_address, (socklen_t *)&client_addr_len);

        if(connect_fd < 0)
        {
            perror("Error Accepting Connection");
        }

        pid = fork();
        if(pid < 0)
        {
            perror("Error Creating Connection");
        }

        if(pid == 0)
        {
            close(socket_fd);
            printf("Connection is Established with the Client...\n");
            read_write(connect_fd);
            printf("Connection closed with the Client...\n");
            close(connect_fd);
            exit(0);
        }
        else
        {
            signal(SIGCHLD, zombie_handler_func);
            close(connect_fd);
        }


    }

    close(socket_fd);
}
