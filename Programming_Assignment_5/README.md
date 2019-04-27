
# Network Simulation


ECEN 602 Network Programming Assignment 5
-------------------------------------------------------------------------------------------------------------------------

Team Number - 8

Member 1 - Amit Rawat (UIN 326005672)

Member 2 - Gautham Srinivasan ( UIN 927008557)

------------------------------------------------------------------------------------------------------------------------
Description:

1. In this assignment NS-2 simulator is used to build following configuration:
   a) Two routers (R1, R2) connected with a 1 Mbps link and 5ms of latency.
   b) Two senders (src1, src2) connected to R1 with 10 Mbps links.
   c) Two receivers (rcv1, rcv2) connected to R2 with 10 Mbps links.
   d) Application sender is FTP over TCP.

2. The simulation below were ran for a period of 400 seconds (ignoring first 100 seconds) with following parameter TCP Version = (TCP SACK | TCP VEGAS)

Case 1:
   > src1-R1 and R2-rcv1 end-2-end delay = 5 ms
   > src2-R1 and R2-rcv2 end-2-end delay = 12.5 ms
Case 2:
   > src1-R1 and R2-rcv1 end-2-end delay = 5 ms
   > src2-R1 and R2-rcv2 end-2-end delay = 20 ms
Case 3:
   > src1-R1 and R2-rcv1 end-2-end delay = 5 ms
   > src2-R1 and R2-rcv2 end-2-end delay = 27.5 ms

--------------------------------------------------------------------------------------------------------------------------
Directory Structure:

ns2.tcl - TCL Script


