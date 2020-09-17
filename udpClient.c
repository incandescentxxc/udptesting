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
    int duration = 10; // in default set it to 10s

    int sockCli = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockCli == -1){
        perror("Client socket create fail: ");
    }


    struct sockaddr_in addrCli;
    memset(&addrCli, 0, sizeof(addrCli));
    addrCli.sin_family = AF_INET;
    addrCli.sin_port = htons(DEST_PORT);
    addrCli.sin_addr.s_addr = inet_addr("172.20.26.31");
    int totalen = sizeof(struct sockaddr_in);

    socklen_t addrlen = sizeof(struct sockaddr);

    //Bind
    if( bind(sockCli ,(struct sockaddr *)&addrCli , addrlen) == -1)
    {
        perror("Client socket bind failed\n");
        return 0;
    }

    if(argc != 1){
        duration = atoi(argv[1]); // in second
    }
    gettimeofday(&t1, NULL);
    gettimeofday(&t2, NULL);
    int counter = 1;
    while(t2.tv_sec - t1.tv_sec <= duration){
        char send_buf[SEND_UNIT];
        sprintf(send_buf, "%d", counter++); // store integer to the buffer
        printf("Client sends: %s\n", send_buf);
        int send_num;
        send_num = sendto(sockCli, send_buf, sizeof(send_buf), 0, (struct sockaddr*)&addrCli,totalen);
        if (send_num < 0){
            perror("sendto error:");
            exit(1);
        }
        gettimeofday(&t2, NULL);
    }
    close(sockCli);
    return 0;
}