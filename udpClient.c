#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "iperf_time.h"
#define DEST_PORT 8000
#define SEND_UNIT 1200 // length of packets sent

// sending a series of packets that are numbered within certain duration
int main(int argc, char *argv[]){

    struct iperf_time before, current, elapsedTime ;
    int packets_to_send = 100000; // in default set it to 100000

    // connect
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
    printf("Client: Connects to 172.16.33.29\n");
    printf("Sending unit: %d\n", SEND_UNIT);

    if(argc != 1){
        packets_to_send = atoi(argv[1]); // in packet number
    }

    iperf_time_now(&before);
    int packetsent = 0;
    while(packetsent < packets_to_send){
        int r;
        //- Header operation
        char send_buf[SEND_UNIT];
        iperf_time_now(&current);
        ++packetsent;
        uint32_t sec, usec, pcount;

		sec = htonl(current.secs);
		usec = htonl(current.usecs);
		pcount = htonl(packetsent);

		memcpy(send_buf, &sec, sizeof(sec));
		memcpy(send_buf + 4, &usec, sizeof(usec));
		memcpy(send_buf + 8, &pcount, sizeof(pcount));
        memset(send_buf + 12, 0, SEND_UNIT - 12);

        r = sendto(sockCli, send_buf, sizeof(send_buf), 0, (struct sockaddr*)&addrCli,totalen);
        if (r < 0){
            perror("sendto error:");
            exit(1);
        }
    }
    close(sockCli);
    iperf_time_now(&current);
    iperf_time_diff(&current, &before, &elapsedTime);
    double duration = iperf_time_in_secs(&elapsedTime);
    int bytes_sent = packets_to_send*SEND_UNIT;
    double throughput = (double)bytes_sent/(1024*1024*duration);
    printf("The duration of sending %d packets lasts %.4fs, throughput is %.4fM/s\n", packets_to_send, duration, throughput);
    return 0;
}