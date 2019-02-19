#include "common.h"

int main(int argc, char * argv[]){
    if(argc < 2) {
        printf("Input Error: Please provide port address as commoand line argument\n");
        exit(1);
    }

    // Variable decleration
    int port = atoi(argv[1]);
    int socket_fd = create_socket();
    struct sockaddr_in server_address, *client_addresses;
    int socket_itr;
    // Maximum number of connection
    int max_client = atoi(argv[2]);
    int client_count = 0;

    // Variables for select
    fd_set set1;
    fd_set set2;
    int max_fd;
    int dup_fd;
    int k;

    // All the user related data
    struct user_data * clients;

    // Initialization
    set_server_address(&server_address, port);
    bind_server(socket_fd, server_address);
    start_listening(socket_fd);

    // Assigning data
    client_addresses = (struct sockaddr_in *) malloc(max_client * sizeof(struct sockaddr_in));
    clients = (struct user_data *) malloc(max_client * sizeof(struct user_data));
    FD_SET(socket_fd, &set1);
    max_fd = socket_fd;


    while(1)
    {
        set2 = set1;
        if(select(max_fd+1, &set2, NULL, NULL, NULL) == -1)
        {
            perror("Select Error: ");
            exit(-1);
        }
        for(socket_itr=0; socket_itr <=max_fd; socket_itr++)
        {
            if(FD_ISSET(socket_itr, &set2))
            {
                if(socket_itr == socket_fd)
                {
                    int new_client_fd = accept_connection(client_addresses, client_count, socket_fd);
                    dup_fd = max_fd;
                    int status = join_message_process(new_client_fd, &client_count, max_client, clients);
                    if(status != -1)
                    {
                        max_fd = new_client_fd > max_fd ? new_client_fd : max_fd;
                        FD_SET(new_client_fd, &set1);
                    	sbcp_message_t * join_message = get_join_message(clients[client_count].user_name);
                        for(k=0; k <= max_fd; k++)
                        {
                            if (FD_ISSET(k, &set1)) {

									if (k != new_client_fd && k != socket_fd) //remove the sender and the listener sockets
                                    {
										if(send(k, (void *) join_message, sizeof(sbcp_message_t), 0) == -1){
											perror("Sending Message Error: ");
											exit(-1);
										}

									}
								}
                        }
                    }
                }
                else
                {
                    broadcast_message(socket_fd, socket_itr, clients, max_fd, &set1, &client_count);
                }

            }
        }
    }

    close(socket_fd);
}
