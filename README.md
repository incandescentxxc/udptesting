# udptesting
The repo for testing the udp latency, loss rate etc.

Note: the file without the suffix is executable file.

<strong> For Loss rate testing, </strong>
udpClient.cpp and udpServer.cpp are used.
client will send a certain number of packets to server side. While server will continuously be in the state of recvfrom.

<strong> For latency measurement, </strong>
udpPower.cpp is used. with the argument 0 being server, and 1 being client.
client will send a packet to server. (Mark starting time) After server recives the packet, the server will send the packet back to the client. 
After the client receives the packet (mark ending time), one iteration will be completed. The delay will be (ending time - starting time) /2.

<strong> For throughput testing, </strong>
it will be (total data size)/total delay

<strong> udplite.cpp is the updlite version of udpPower.cpp</strong>

<br>
To run the codes in udpPower.cpp and udplite.cpp, do two things before running:
1. modify the cpu_frequency to that of your cpu (To see the cpu frequency in linux: watch -n.1 "cat /proc/cpuinfo | grep \"^[c]pu MHz\"" )
<br>
2. use: taskset --cpu-list 1,2,3 my_command  to restrict the program/process to run in cpu 1,2,3. (here the case is that you need to run it in one cpu core).


<strong> FINDINGS </strong>
1. Loss rate will increase with the distance increasing.
2. Loss rate will increase with the size of datagram increasing. (But fix it to 1400 bytes, it's not an issue any more.)

