#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#define DEST_PORT 8000
#define SEND_UNIT 1200 // length of packets sent

// sending a series of packets that are numbered within certain duration
int main(int argc, char *argv[]){

    struct timeval t1, t2;
    double elapsedTime;
    int packets_to_send = 100000; // in default set it to 100000

    int sockCli = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockCli == -1){
        perror("Client socket create fail: ");
    }


    struct sockaddr_in addrCli;
    memset(&addrCli, 0, sizeof(addrCli));
    addrCli.sin_family = AF_INET;
    addrCli.sin_port = htons(DEST_PORT);
    addrCli.sin_addr.s_addr = inet_addr("172.16.33.29");
    int totalen = sizeof(struct sockaddr_in);

    socklen_t addrlen = sizeof(struct sockaddr);

    if(argc != 1){
        packets_to_send = atoi(argv[1]); // in packet number
    }
    gettimeofday(&t1, NULL);
    int packetsent = 0;
    while(packetsent < packets_to_send){
        //TODO - Header modification needed
        char send_buf[SEND_UNIT];
        sprintf(send_buf, "%d", ++packetsent); // store integer to the buffer
        int send_num;
        send_num = sendto(sockCli, send_buf, sizeof(send_buf), 0, (struct sockaddr*)&addrCli,totalen);
        if (send_num < 0){
            perror("sendto error:");
            exit(1);
        }
        if((packetsent % 1000) == 0){
            printf("Client sends: %s\n", send_buf);
            usleep(1000);
        }
    }
    close(sockCli);
    gettimeofday(&t2, NULL);
    int duration = 1000000*(t2.tv_sec - t1.tv_sec) + (t2.tv_usec-t1.tv_usec);
    printf("The duration of sending packets lasts %.4fs\n", (float)duration/1000000);
    return 0;
}