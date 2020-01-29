#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#define DEST_PORT 8000

int main(){

    // //set an interval for sending rate control
    // struct itimerval timer;
    // timer.it_interval.tv_sec = timer.it_value.tv_sec = 0;
    // timer.it_interval.tv_usec = timer.it_value.tv_usec = 10;   /* 10 microseconds */
    // if (setitimer(ITIMER_REAL, &timer, 0) < 0) {
    // perror("setitimer");
    // exit(1); }
    // sigset_t alarm_sig;
    // int signum;
    // sigemptyset(&alarm_sig);
    // sigaddset(&alarm_sig, SIGALRM);
    

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

    
    int counter = 1;
    while(counter <= 100000){
        //send every 10ms
        // sigwait(&alarm_sig, &signum); /* wait until the next signal */
        if(counter== 100000){
            char newbuf[1400] = "Finish!";
            printf("Client sends: %s\n", newbuf);
            int send_num;
            send_num = sendto(sockCli, newbuf, sizeof(newbuf), 0, (struct sockaddr*)&addrCli,totalen);
            if (send_num < 0){
                perror("sendto error:");
                exit(1);
            }
        }
        else{
            char sendbuf[1400];
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
    close(sockCli);
    return 0;
}