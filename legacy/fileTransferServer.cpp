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
#define SERV_PORT 8000
#define PACK_SIZE 1400
using namespace std;

int udpReceive(){
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
    ofstream oufile;
    oufile.open("video1");
    char recvbuf[PACK_SIZE];
    double lossRate[100000];
    int counter = 0;
    while(1){
        int recv_num = recvfrom(sockSer, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*)&addrCli,(socklen_t *)&totalen);
        if(recv_num < 0){
            perror("recv from error:");
            exit(1);
        }
        if(!strcmp(recvbuf,"Finish!")){
            printf("This is the last one!\n");
            break;
        }
        oufile.write(recvbuf, PACK_SIZE); 
    }

    double rate = counter/1000.0; // percentage
    printf("The counter is %d\n", counter);
    oufile.close();
    close(sockSer);
    return 0;
    

}