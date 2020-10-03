#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "iperf_time.h"
#define DEST_PORT 8000
#define SEND_UNIT 1024 // length of packets sent

// COMMAND: ./udpclient [udp_proto] [num_stream] [num_packets]

int main(int argc, char *argv[])
{

    struct iperf_time before, current, elapsedTime;
    int packets_to_send = 100000; // in default set it to 100000
    int num_streams = 10;
    int proto_num = atoi(argv[1]); // required field
    // connect
    char *server_addr = "192.168.1.133";
    // choose protocol to use
    int sockCli;
    switch (proto_num)
    {
    case 1: // normal udp
        sockCli = socket(AF_INET, SOCK_DGRAM, 0);
        printf("Default UDP is used\n");
        break;
    case 2: // udp lite
        sockCli = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDPLITE);
        printf("UDPLite is used\n");
    }
    if (sockCli == -1)
    {
        perror("Client socket create fail: ");
    }
    struct sockaddr_in addrCli;
    memset(&addrCli, 0, sizeof(addrCli));
    addrCli.sin_family = AF_INET;
    addrCli.sin_port = htons(DEST_PORT);
    addrCli.sin_addr.s_addr = inet_addr(server_addr);
    int totalen = sizeof(struct sockaddr_in);

    socklen_t addrlen = sizeof(struct sockaddr);
    printf("Client: Connects to %s\n", server_addr);
    printf("Sending unit: %d\n", SEND_UNIT);

    if (argc != 1)
    {
        num_streams = atoi(argv[2]);
        packets_to_send = atoi(argv[3]); // in packet number
    }
    int packetsent, stream_id;
    int bytes_sent = packets_to_send * SEND_UNIT;
    double *duration = (double *)malloc(num_streams * sizeof(double));
    double *throughput = (double *)malloc(num_streams * sizeof(double));
    for (stream_id = 1; stream_id < num_streams + 1; stream_id++) 
    {
        printf("Stream %d starts, %d packets to be sent\n", stream_id, packets_to_send);
        iperf_time_now(&before);
        packetsent = 0;
        while (packetsent < packets_to_send)
        {
            int r;
            //- Header operation
            char send_buf[SEND_UNIT];
            iperf_time_now(&current);
            ++packetsent;
            uint32_t sec, usec, pcount, stream_id_nl;
            stream_id_nl = htonl(stream_id);
            sec = htonl(current.secs);
            usec = htonl(current.usecs);
            pcount = htonl(packetsent);

            memcpy(send_buf, &stream_id_nl, sizeof(stream_id_nl));
            memcpy(send_buf + 4, &sec, sizeof(sec));
            memcpy(send_buf + 8, &usec, sizeof(usec));
            memcpy(send_buf + 12, &pcount, sizeof(pcount));
            memset(send_buf + 16, 0, SEND_UNIT - 16);
            r = sendto(sockCli, send_buf, sizeof(send_buf), 0, (struct sockaddr *)&addrCli, totalen);
            if (r < 0)
            {
                perror("sendto error:");
                exit(1);
            }
        }
        iperf_time_now(&current);
        iperf_time_diff(&current, &before, &elapsedTime);
        duration[stream_id - 1] = iperf_time_in_secs(&elapsedTime);
        throughput[stream_id - 1] = (double)bytes_sent / (1024 * 1024 * duration[stream_id - 1]);
        printf("Stream %d lasts %.4fs, throughput is %.4fM/s\n", stream_id, duration[stream_id - 1], throughput[stream_id - 1]);
        usleep(100000);
    }
    close(sockCli);

    double total_duration = 0;
    double avg_throughput = 0;
    for (int i = 0; i < num_streams; i++)
    {
        total_duration += duration[i];
        avg_throughput += throughput[i];
    }
    avg_throughput /= num_streams;
    printf("Total %d packets have been sent, lasting %.4fs, throughput is %.4fM/s\n", packets_to_send * num_streams, total_duration, avg_throughput);
    free(duration);
    free(throughput);
    return 0;
}