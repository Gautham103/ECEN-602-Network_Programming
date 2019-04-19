
# Simple HTTP Proxy


ECEN 602 Network Programming Assignment 4
-------------------------------------------------------------------------------------------------------------------------

Team Number - 8

Member 1 - Amit Rawat (UIN 326005672)

Member 2 - Gautham Srinivasan ( UIN 927008557)

------------------------------------------------------------------------------------------------------------------------
Description:

In this assignment, we have implement a simple HTTP proxy server and HTTP command line client. This implementation used HTTP/1.0, which is specified in RFC 1945.

Amit Rawat: Implemented client.c and some section of helper.c file.
Gautham Srinivasan: Implemented server.c and some section of helper.c file.

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


