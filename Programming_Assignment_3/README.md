
# TFTP Server Project


ECEN 602 Network Programming Assignment 3
-------------------------------------------------------------------------------------------------------------------------

Team Number - 8

Member 1 - Amit Rawat (UIN 326005672)

Member 2 - Gautham Srinivasan ( UIN 927008557)

------------------------------------------------------------------------------------------------------------------------
Description:

In this project, we have implemented a UDP based TFTP protocol for file transfer. The server can transfer file in both modes ascii and binary modes. It also supports wrap around to transfer files above 32MB as a basic stop and wait protocol. A bonus feature is also implemented which support WRQ request which allows writing the file back to the server.

Amit Rawat: Implemented some section of server.c and helper.c file.
Client.C, IPv6, IDLE feature and test cases are implemented by Gautham Srinivasan.

--------------------------------------------------------------------------------------------------------------------------
Directory Structure:

bin - contains the binary client and server

include - contains the file common.h

obj - contains the object files of server.c, common.c, and client.c.

src - contains the source file.

--------------------------------------------------------------------------------------------------------------------------
Functions Implemented:

1) create_socket - create a new socket file descriptor.
2) set_server_address - set the server address.
3) bind_server, bind_server_ipv6 - bind the socket
4) start_listening - it make server to listen from the socket.
5) set_server_address,set_server_address_ipv6 - assign the ip and port passed as the command line argument.
6) accept_connection - accept connections from client and return file descriptor.
7) send_message - send the message.
8) send_data - send data message.
9) send_ack - send ACK message.
10) send_error_message - send error message.
11) broadcast_message - broadcast the message from the client to all the other clients.
12) receive_message - receive message.
13) get_mode - Return the mode of the file.
14) zombie_handler_func - avoid zombie process.
15) handle_message - handle the message.

--------------------------------------------------------------------------------------------------------------------------
Running the code:
1. sudo apt-get install tftp     - for installing tftp
2. Go to the directory ./Programming_Assignment_3
3. Run the command: make clean
4. Run the command: make all
5. Go to directory ./bin
6. For starting server: ./server IP_ADDRESS PORT
7. Put all the files to be sent to client in the ./bin directory
8. 


