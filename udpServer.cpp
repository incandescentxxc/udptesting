#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<stdlib.h>
# include <iostream>
# include <fstream>
# include <bitset>
using namespace std;

#define SERV_PORT 8000
int main(){
    // create file descriptor. declare IPv4, UDP protocol
    int sockSer = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockSer == -1){
        perror("socket");
    }
    //declare addresses
    struct sockaddr_in addrSer;// server address
    struct sockaddr_in addrCli;// client address
    memset(&addrSer, 0, sizeof(addrSer)); //data initialization as 0
    addrSer.sin_family = AF_INET; // set IP connection mode
    addrSer.sin_port = htons(SERV_PORT); // set server port
    addrSer.sin_addr.s_addr = INADDR_ANY; // allow connect to all local ip address 
    int totalen = sizeof(struct sockaddr_in);
    
    // bind fp to the server's address
    socklen_t addrlen = sizeof(struct sockaddr);
    int res = bind(sockSer, (struct sockaddr*)&addrSer, addrlen);
    if (res == -1){
        perror("bind error:");
    }
    // change buffer size to 3M
    int n = 1024 * 1024 * 3;
    setsockopt(sockSer, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n));

    char recvbuf[1400];
    double lossRate[100000];
    int counter = 0;
    ofstream oufile;
    oufile.open("udp.log");
    cout << "waiting for packets..." << endl;
    while(1){
        int recv_num = recvfrom(sockSer, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*)&addrCli,(socklen_t *)&totalen);
        if(recv_num < 0){
            perror("recv from error:");
            exit(1);
        }
        recvbuf[recv_num] = '\0'; //forced to have a null terminator in the end
        // printf("Server received %d bytes of data: %s\n", recv_num, recvbuf);
        if(!strcmp(recvbuf, "Change")){
            int n = 1024 * 256;
            setsockopt(sockSer, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n));
            cout << "Buffer size from 3M to 256k" << endl;
            oufile << "Buffer size from 3M to 256k";
            
        }
        if(!strcmp(recvbuf,"Finish!")){
            printf("This is the last one!\n");
            break;
        }
        counter ++;
        int realNumRecv = atoi(recvbuf);
        string serial = to_string(realNumRecv)  + "\n";
        // lossRate[counter-1] = counter/(double)realNumRecv*100;
        oufile << serial;
        // printf("Counter number is %d\n", counter);
        // printf("Real time receving rate: %.4f%%\n",lossRate[counter-1]);
    }
    oufile.close();
    double rate = counter/1500.0; // percentage
    printf("The counter is %d\n", counter);

    printf("The receiving rate is %.4f%%\n",rate);
    close(sockSer);
    return 0;
    
}