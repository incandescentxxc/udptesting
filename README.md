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

