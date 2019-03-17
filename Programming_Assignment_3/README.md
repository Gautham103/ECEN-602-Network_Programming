
# TFTP Server Project


ECEN 602 Network Programming Assignment 3
-------------------------------------------------------------------------------------------------------------------------

Team Number - 8

Member 1 - Amit Rawat (UIN 326005672)

Member 2 - Gautham Srinivasan ( UIN 927008557)

------------------------------------------------------------------------------------------------------------------------
Description:

In this project, we have implemented a UDP based TFTP protocol for file transfer. The server can transfer file in both modes ascii and binary modes. It also supports wrap around to transfer files above 32MB as a basic stop and wait protocol. A bonus feature is also implemented which support WRQ request which allows writing the file back to the server.

Server.c, helper.c and makefile are implemented by Amit Rawat.
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
3) bind_server - bind the socket
4) start_listening - it make server to listen from the socket.
5) set_server_address - assign the ip and port passed as the command line argument.
6) accept_connection - accept connections from client and return file descriptor.
7) send_ack_message - send the ack message to client.
8) send_nack_message - send the nack message to client.
9) join_message_process - processes the join message sent by the client.
10) remove_client - remove the client from the list of user.
11) broadcast_message - broadcast the message from the client to all the other clients.
12) get_join_message - generate the message sending to other client about a newly joined client.
13) get_hung_message - generate the message when a client leave chat room.
--------------------------------------------------------------------------------------------------------------------------
File Name: server.c
Usage:
1) It creates a socket and starts listening on that socket.
2) It processes the connection from the client.
3) If the connection is accepted then a ACK message is sent to the client.
4) If the connection is rejected then a NACK message is sent to the client.
5) It also forward the message received from one client to another client.
6) After client exits it closes connection with the client.
---------------------------------------------------------------------------------------------------------------------------
File Name: client.c
Usage:
1) It creates a socket and connects to the server using IP address and port number.
2) It gets the input as stdin from user and sends it to the server and also reads data from the server.


---------------------------------------------------------------------------------------------------------------------------
File Name: helper.c
Usage:
It contains all the helper function which are used in server code.

---------------------------------------------------------------------------------------------------------------------------
File Name: common.h
Usage:
It contains all the function declaration and header files used across the code.

--------------------------------------------------------------------------------------------------------------------------
Running the code:
1. Go to the directory ./Programming_Assignment_2
2. Run the command: make clean
3. Run the command: make all
4. Go to directory ./bin
5. For starting server: ./server IP_ADDRESS PORT MAXIMUM_USER
6. To start the client: ./client CLIENT_NAME IP_ADDRESS PORT


