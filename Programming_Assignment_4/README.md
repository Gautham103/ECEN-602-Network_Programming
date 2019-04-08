
# Simple HTTP Proxy


ECEN 602 Network Programming Assignment 4
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
Running the code:
1. Go to the directory ./Programming_Assignment_4
2. Run the command: make clean
3. Run the command: make all
4. Go to directory ./bin
5. For starting http server: ./http IP_ADDRESS PORT
6. To start the client: ./client IP_ADDRESS PORT WEB_ADDRESS
   e.g. ./client 127.0.0.1 1234 www.<domain>.com
   
   Note: Do not use http://<domain>.com for web address rather use www.<domain>.com


