#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include "iperf_time.h"
#define SERV_PORT 8000
#define RECV_UNIT 1200 // length of packets sent

int main(int argc, char *argv[])
{
    int df_duration = 3; // in default 3s
    // create file descriptor. declare IPv4, UDP protocol
    int sockSer = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockSer == -1)
    {
        perror("server socket creation fails: ");
    }
    //declare addresses
    struct sockaddr_in addrSer;           // server address
    struct sockaddr_in addrCli;           // client address
    memset(&addrSer, 0, sizeof(addrSer)); //data initialization as 0
    addrSer.sin_family = AF_INET;         // set IP connection mode
    addrSer.sin_port = htons(SERV_PORT);  // set server port
    addrSer.sin_addr.s_addr = INADDR_ANY; // allow connect to all local ip address
    int totalen = sizeof(struct sockaddr_in);

    // bind fp to the server's address
    socklen_t addrlen = sizeof(struct sockaddr);
    int res = bind(sockSer, (struct sockaddr *)&addrSer, addrlen);
    if (res == -1)
    {
        perror("server bind error:");
    }
    // change buffer size to 3M
    // int n = 1024 * 1024 * 5;
    // setsockopt(sockSer, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n));

    char recvbuf[RECV_UNIT];
    int first_packet = 0;
    int packets_count = 0;
    uint32_t sec, usec, pcount;
    uint64_t bytes_received, acc_delay;
    struct iperf_time sent_time, arrival_time, temp_time, first_arr_time;
    iperf_time_now(&first_arr_time);
    iperf_time_now(&arrival_time);
    iperf_time_diff(&arrival_time, &first_arr_time, &temp_time);
    while (iperf_time_in_secs(&temp_time) < df_duration)
    {
        int r = recvfrom(sockSer, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&addrCli, (socklen_t *)&totalen);
        if (r < 0)
        {
            perror("recv from error:");
            exit(1);
        }
        if (bytes_received == 0)
        {
            iperf_time_now(&first_arr_time);
            first_packet = 1;
        }
        packets_count++;
        bytes_received += r;
        memcpy(&sec, recvbuf, sizeof(sec));
        memcpy(&usec, recvbuf + 4, sizeof(usec));
        memcpy(&pcount, recvbuf + 8, sizeof(pcount));
        sec = ntohl(sec);
        usec = ntohl(usec);
        pcount = ntohl(pcount);
        sent_time.secs = sec;
        sent_time.usecs = usec;
        iperf_time_now(&arrival_time);
        iperf_time_diff(&arrival_time, &sent_time, &temp_time);
        acc_delay += iperf_time_in_usecs(&temp_time);
    }
    close(sockSer);
    // get average loss rate
    
    double aver_loss_rate = packets_count / 10000;

    FILE *fp;
    fp = fopen("test.txt", "a");
    fprintf(fp, "Average loss rate is %.4lf%%\n", aver_loss_rate);
    fclose(fp);
    // printf("The average loss rate is %.4lf%%\n",aver_loss_rate);

    return 0;
}