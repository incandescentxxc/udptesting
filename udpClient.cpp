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
    addrCli.sin_addr.s_addr = inet_addr("127.0.0.1");
    int totalen = sizeof(struct sockaddr_in);


    socklen_t addrlen = sizeof(struct sockaddr);

    char sendbuf[30] = "Hey, I am client.";
    int counter = 0;
    while(counter <= 1000000){
        if(counter==1000000){
            char newbuf[30] = "Finish!";
            printf("Client sends: %s\n", newbuf);
            int send_num;
            send_num = sendto(sockCli, newbuf, sizeof(newbuf), 0, (struct sockaddr*)&addrCli,totalen);
            if (send_num < 0){
                perror("sendto error:");
                exit(1);
            }
        }
        else{
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