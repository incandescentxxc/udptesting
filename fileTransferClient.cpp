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
    infile.open(fileName, ios::binary | ios::ate); // read the file from the end
    int length = infile.tellg();
    infile.close();
    length = length / PACK_SIZE + 1;
    return length;
}

//cut the file into pieces and store them in a 2d array
char** scissors(string fileName) {
    int bufNum = getBufNum(fileName);
    char **buf = new char* [bufNum];
    ifstream infile;
    // open with binary form
    infile.open(fileName, ios::binary | ios::ate);
    // locate to the beginning of the file
    infile.seekg(0, ios::beg);
    if(infile.is_open()){
        // read 1400 bytes
        for(int i = 0; i< bufNum; i++){
            buf[i] = new char[PACK_SIZE];
            infile.read(buf[i], PACK_SIZE);
        }
    }
    infile.close();
    return buf;
}

//for testing whether it is correct when cutting files, tested to be correct
int sticker(char **buf, string fileName){
    ofstream oufile;
    oufile.open("video.mp4");
    int num = getBufNum(fileName);
    for(int i = 0; i<num; i++){
        oufile.write(buf[i], PACK_SIZE);
    }
    oufile.close();
    return 0;
}

int udpSend(string fileName, char** buf, int mode){
    //mode to indicate the protocol used, 0 for UDP default, 1 for UDP lite
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
        sleep(0.001);// wait one millisecond
    }
    
    cout << "Finish sending" << endl;
    char* lastMsg = "Finish!";
    int send_num = sendto(sockCli, lastMsg, sizeof(lastMsg), 0, (struct sockaddr*)&addrCli,totalen);
    close(sockCli);
    return 0;
}


int main(){
    char **contents = scissors("Videos tested/Trial1_55M.mp4");
    udpSend("Videos tested/Trial1_55M.mp4", contents, 0);
    // sticker(contents, "Videos tested/Trial1_55M.mp4");
    
}
