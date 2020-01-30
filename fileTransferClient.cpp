#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
# include <iostream>
# include <fstream>
# include <bitset>
#define DEST_PORT 8000
#define PACK_SIZE 1400
using namespace std;

int getBufNum(string fileName){
    ifstream infile;
    infile.open(fileName, ios::binary | ios::ate);
    int length = infile.tellg();
    infile.close();
    length = length / PACK_SIZE + 1;
    return length;
}

char** scissors(string fileName) {
    int bufNum = getBufNum(fileName);
    char **buf = new char* [bufNum];
    ifstream infile;
    infile.open(fileName, ios::binary | ios::ate);
    infile.seekg(0, ios::beg);
    if(infile.is_open()){
        for(int i = 0; i< bufNum; i++){
            buf[i] = new char[PACK_SIZE];
            infile.read(buf[i], PACK_SIZE);
        }
    }
    infile.close();
    return buf;
}

int udpSend(string fileName, char** buf){
     int sockCli = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockCli == -1){
        perror("socket");
    }


    struct sockaddr_in addrCli;
    memset(&addrCli, 0, sizeof(addrCli));
    addrCli.sin_family = AF_INET;
    addrCli.sin_port = htons(DEST_PORT);
    addrCli.sin_addr.s_addr = inet_addr("192.168.1.42");
    int totalen = sizeof(struct sockaddr_in);


    socklen_t addrlen = sizeof(struct sockaddr);



    int limit = getBufNum(fileName);
    cout << "Totally " << limit << " packets will be sended." << endl;
    for(int i = 0; i < limit; i++){
        int send_num = sendto(sockCli, buf[i], sizeof(buf[i]), 0, (struct sockaddr*)&addrCli,totalen);
        if (send_num < 0){
                perror("sendto error:");
                exit(1);
        }
    }
    
    cout << "Finish sending" << endl;
    close(sockCli);
    return 0;
}


int main(){
    char **contents = scissors("Videos tested/Trail2_110M.mp4");
    udpSend("Videos tested/Trail2_110M.mp4", contents);
    
}
