
# TCP Echo Server and Client


ECEN 602 Network Programming Assignment 1

-------------------------------------------------------------------------------------------------------------------------

Team Number - 8

Member 1 - Amit Rawat (UIN 326005672)

Member 2 - Gautham Srinivasan ( UIN 927008557)

------------------------------------------------------------------------------------------------------------------------
Description:

In this project, we have implemented an RFC 862 echo protocol. The server program accepts requests from the clients, and whatever the client sends to the server is echoed back to the client. When the client terminates, the server closes the connection with the client.

Server and makefile are implemented by Amit Rawat.
Client, read/write functions and test cases are implemented by Gautham Srinivasan.

--------------------------------------------------------------------------------------------------------------------------
Directory Structure:

bin - contains the binary echo(client) and echos(server)

include - contains the file common.h

obj - contains the object files of server.c, common.c, and client.c.

src - contains the source file.

--------------------------------------------------------------------------------------------------------------------------
Functions Implemented:
1) writen - write data on a socket.
2) iReadLine - read data from the socket.
3) create_socket - create a new socket file descriptor.
4) set_server_address - set the server address.
5) bind_server - bind the socket
6) start_listening - it make server to listen from the socket.
7) zombie_handler_func - function for cleaning zombie process.
8) read_write - read from socket and write to the socket.

--------------------------------------------------------------------------------------------------------------------------
File Name: server.c
Usage:
1) It creates a socket and starts listening on that socket.
2) It accepts connection from the client. After accepting connection, it forks a new process to serve the client. It also handles zombie processes.
3) After client exits it closes connection with the client.
---------------------------------------------------------------------------------------------------------------------------
File Name: client.c
Usage:
1) It creates a socket and connects to the server using IP address and port number.
2) It gets the input as stdin from user and sends it to the server and also reads data from the server.


---------------------------------------------------------------------------------------------------------------------------
File Name: helper.c

Usage:

It contains all the helper function which are used in client and server code.

---------------------------------------------------------------------------------------------------------------------------
File Name: common.h

Usage:

It contains all the function declaration and header files used across the code.

--------------------------------------------------------------------------------------------------------------------------
Running the code:
1. Go to the directory
2. First run: make clean
3. Compile code and generate binary: make -f makefile
4. To run client: ./bin/echo IP_ADDRESS PORT        (Provide the IP address = 127.0.0.1 and port number as command Line)
5. To run server: ./bin/echos PORT                  (Provide port number as command Line)


