#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<stdlib.h>
#include <sys/time.h>
#define SERV_PORT 8000
#define RECV_UNIT 1200 // length of packets sent

int main(int argc, char *argv[]){
    struct timeval t1,t2;
    int duration = 5; // in default 5s
    // create file descriptor. declare IPv4, UDP protocol
    int sockSer = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockSer == -1){
        perror("server socket creation fails: ");
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
        perror("server bind error:");
    }
    // change buffer size to 3M
    // int n = 1024 * 1024 * 5;
    // setsockopt(sockSer, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n));
    if(argc > 1){
        duration = atoi(argv[1]);
    }
    float interval = 0.1; // sampling interval
    int num_data = (int) duration / interval;
    char recvbuf[RECV_UNIT];
    double *loss_rate = (double *) malloc(num_data*sizeof(double) + 100);
    int count = 0;
    int counter_data = 0;
    gettimeofday(&t1, NULL);
    gettimeofday(&t2, NULL);
    int init_flag = 0;
    int num_recv;
    while(t2.tv_sec - t1.tv_sec <= duration){
        int recv_num = recvfrom(sockSer, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*)&addrCli,(socklen_t *)&totalen);
        if(!init_flag){ // after receiving the first packet, set the starting time formally
            init_flag = 1;
            gettimeofday(&t1, NULL);
        }
        if(recv_num < 0){
            perror("recv from error:");
            exit(1);
        }
        count++;
        gettimeofday(&t2, NULL);
        // for every interval, records the data
        if(t2.tv_sec - t1.tv_sec >= counter_data * interval){ //100ms
            num_recv = atoi(recvbuf);
            loss_rate[counter_data++] = (1- count/(double)num_recv)*100;
            printf("packet number is %d, count number is %d\n", num_recv, count);
        }

    }
    close(sockSer);
    // get average loss rate
    double acc;
    for(int i = 0; i < num_data; i++){
        printf("loss rate is %.4lf%%\n",loss_rate[i]);
        acc += loss_rate[i];
    }
    free(loss_rate);
    double aver_loss_rate = acc/num_data;

    printf("The average loss rate is %.4lf%%\n",aver_loss_rate);
    
    return 0;
    
}