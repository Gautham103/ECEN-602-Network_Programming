#TCL script for NS2
if { $argc != 2 } {
    puts "Please enter correct systax: ns2.tcl <TCP_tcp_flavor> <case_number>"
}
    
    set input_flavor [lindex $argv 0]
    set case_number [lindex $argv 1]
    
#Checking case number is valid
    if {$case_number > 3 || $case_number < 1} {
        puts "invalid case entered $case_number"
        exit
    }
    
    global tcp_flavor, new_delay
    set new_delay 0
    
#Checking tcp_flavor is valid
    if {$input_flavor == "SACK" || $input_flavor == "sack"} {
        set tcp_flavor "Sack1"
    } elseif {$input_flavor == "VEGAS" || $input_flavor == "vegas"} {
        set tcp_flavor "Vegas"
    } else {
        puts "Invalid TCP type entered - $input_flavor"
        exit
    }
	
#Choose the input delay 
    switch $case_number {
        global new_delay
        1 {set new_delay "12.5ms"}
        2 {set new_delay "20.0ms"}
        3 {set new_delay "27.5ms"}
    }
        
#Store data in output files
    set file1 [open out1.tr w]
    set file2 [open out2.tr w]
    
#setup new simulator
    set ns [new Simulator]
    set file "out_$input_flavor$case_number"
    
#open tracefile
    set tracefile [open out.tr w]
    $ns trace-all $tracefile
    
#open NAM tracefile
    set namfile [open out.nam w]
    $ns namtrace-all $namfile
    
    set throughput1 0
    set throughput2 0
    set counter 0
    
#Node declaration and color assignment 
    set src1 [$ns node]
    set src2 [$ns node]
    set rou1 [$ns node]
    set rou2 [$ns node]
    set rcv1 [$ns node]
    set rcv2 [$ns node]
    
# Assign data flow colors
    $ns color 1 Green
    $ns color 2 Red
    
#Defining TCP Agents for connection and color codes assignment
    set tcp1 [new Agent/TCP/$tcp_flavor]
    set tcp2 [new Agent/TCP/$tcp_flavor]
    $ns attach-agent $src1 $tcp1
    $ns attach-agent $src2 $tcp2
    
    $tcp1 set class_ 1
    $tcp2 set class_ 2

#Defining TCP sink connection
    set sinkA [new Agent/TCPSink]
    set sinkB [new Agent/TCPSink]
    $ns attach-agent $rcv1 $sinkA
    $ns attach-agent $rcv2 $sinkB
    
    $ns connect $tcp1 $sinkA
    $ns connect $tcp2 $sinkB
    
#Defining Links
    $ns duplex-link $rou1 $rou2 1.0Mb 5ms DropTail
    $ns duplex-link $src1 $rou1 10.0Mb 5ms DropTail
    $ns duplex-link $rcv1 $rou2 10.0Mb 5ms DropTail
    $ns duplex-link $src2 $rou1 10.0Mb $new_delay DropTail
    $ns duplex-link $rcv2 $rou2 10.0Mb $new_delay DropTail
    
#Defining Nam interfaces
    $ns duplex-link-op $rou1 $rou2 orient right
    $ns duplex-link-op $src1 $rou1 orient right-down
    $ns duplex-link-op $src2 $rou1 orient right-up
    $ns duplex-link-op $rou2 $rcv1 orient right-up
    $ns duplex-link-op $rou2 $rcv2 orient right-down
    
#Creating FTP traffic on top of TCP connection
    set ftp1 [new Application/FTP]
    set ftp2 [new Application/FTP]
    
    $ftp1 attach-agent $tcp1
    $ftp2 attach-agent $tcp2
    

#record procedure to actually write data to output files
    proc record {} {
        global sinkA sinkB file1 file2 throughput1 throughput2 counter
        
#NS simulator
        set ns [Simulator instance]
        
#set time after which procedure will be called again
        set time 0.5

#get current time
        set now [$ns now]
        
#traffic received by sinks
        set bandwidth1 [$sinkA set bytes_]
        set bandwidth2 [$sinkB set bytes_]
               
# calculate the bandwidth (in MBit/s) and write it to the files
        puts $file1 "$now [expr $bandwidth1/$time*8/1000000]"
        puts $file2 "$now [expr $bandwidth2/$time*8/1000000]"
	if { $now >= 100 } {
	set throughput1 [expr $throughput1+ $bandwidth1/$time*8/1000000 ]
	set throughput2 [expr $throughput2+ $bandwidth2/$time*8/1000000 ]
        }
	set counter [expr $counter + 1]
        
#reset bytes_ on the sinks
        $sinkA set bytes_ 0
        $sinkB set bytes_ 0
        
# re-schedule the procedure
        $ns at [expr $now+$time] "record"
    }
    
#Finish procedure for terminating simulations
    proc finish {} {
        global ns nf tracefile namfile file throughput1 throughput2 counter

        $ns flush-trace
        puts "Avg throughput for Src1=[expr $throughput1/($counter-200)] MBits/sec\n"
        puts "Avg throughput for Src2=[expr $throughput2/($counter-200)] MBits/sec\n"
        puts "The ratio src1/src2 = [expr $throughput1/($throughput2)]\n"
	close $tracefile
        close $namfile
        exec nam out.nam &
        exit 0
    }

#calling record procedure to record simulation
    $ns at 0 "record"
    $ns at 0 "$src1 label Src1"
    $ns at 0 "$src2 label Src2"
    $ns at 0 "$rou1 label Router1"
    $ns at 0 "$rou2 label Router2"
    $ns at 0 "$rcv1 label Rcv1"
    $ns at 0 "$rcv2 label Rcv2"
    $ns at 0 "$ftp1 start"
    $ns at 0 "$ftp2 start"
    $ns at 400 "$ftp1 stop"
    $ns at 400 "$ftp2 stop"
    	
#finish procedure
    $ns at 400 "finish"
    
    $ns color 1 Green
    $ns color 2 Red
    
    set tf1 [open "$file-S1.tr" w]
    $ns trace-queue  $src1  $rou1  $tf1
    
    set tf2 [open "$file-S2.tr" w]
    $ns trace-queue  $src2  $rou1  $tf2
    
    $ns run
