#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#define DEST_PORT 8000
int main(){
    int sockCli = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockCli == -1){
        perror("socket");
    }

    struct sockaddr_in addrCli;
    memset(&addrCli, 0, sizeof(addrCli));
    addrCli.sin_family = AF_INET;
    addrCli.sin_port = htons(DEST_PORT);
    addrCli.sin_addr.s_addr = inet_addr("192.168.2.188");
    int totalen = sizeof(struct sockaddr_in);


    socklen_t addrlen = sizeof(struct sockaddr);

    
    int counter = 1;
    while(counter <= 100000){
        if(counter== 100000){
            char newbuf[128] = "Finish!";
            printf("Client sends: %s\n", newbuf);
            int send_num;
            send_num = sendto(sockCli, newbuf, sizeof(newbuf), 0, (struct sockaddr*)&addrCli,totalen);
            if (send_num < 0){
                perror("sendto error:");
                exit(1);
            }
        }
        else{
            char sendbuf[128];
            sprintf(sendbuf, "%d", counter);
            printf("Client sends: %s\n", sendbuf);
            int send_num;
            send_num = sendto(sockCli, sendbuf, sizeof(sendbuf), 0, (struct sockaddr*)&addrCli,totalen);
            if (send_num < 0){
                perror("sendto error:");
                exit(1);
            }

        }
        counter++;
    }
    printf("counter is %d",counter);
    close(sockCli);
    return 0;
}