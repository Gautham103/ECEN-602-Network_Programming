#include "common.h"

int main(int argc, char * argv[]){
    // Throw error if Port number is not provided
    if(argc < 2) {
        printf("Input Error: Please provide port address as commoand line argument\n");
        exit(1);
    }

    int port = atoi(argv[1]); // Port number obtained from command line

    int client_addr_len;
    int socket_fd = create_socket();
    int pid, connect_fd;
    struct sockaddr_in server_address, client_address;

    
    set_server_address(&server_address, port); // Initialize server addresses and port
    bind_server(socket_fd, server_address);    // Bind to address and port
    start_listening(socket_fd);                // Start listening on socket
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
            read_write(connect_fd);                                   // read from socket and write to socket
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
