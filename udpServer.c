#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include "iperf_time.h"
#define SERV_PORT 8000
#define RECV_UNIT 1024 // length of packets sent

int main(int argc, char *argv[])
{
    int num_streams = 10;         // in default receive 10 streams
    int proto_id = atoi(argv[1]); // required, protocol id
    if (argv[2])
    {
        num_streams = atoi(argv[2]);
    }
    int df_duration = 5; // in default 5s
    // create file descriptor. declare IPv4, UDP protocol
    int sockSer;
    switch (proto_id)
    {
    case 1: // udp
        sockSer = socket(AF_INET, SOCK_DGRAM, 0);
        break;
    case 2: // udp lite
        sockSer = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDPLITE);
    }

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
    uint32_t sec, usec, pcount, signal;
    struct iperf_time sent_time, arrival_time, temp_time, first_arr_time;
    double transit, d;
    iperf_time_now(&first_arr_time);
    iperf_time_now(&arrival_time);
    iperf_time_diff(&arrival_time, &first_arr_time, &temp_time);
    int stream_id = 1;
    int *first_packet = (int *)malloc(num_streams * sizeof(int));  // indicate whether first packet has been recv for each stream
    int *packets_count = (int *)malloc(num_streams * sizeof(int)); // highest number received so far
    int *loss_num = (int *)malloc(num_streams * sizeof(int));      // lost packet in each stream
    int *out_of_order_pkt_num = (int *)malloc(num_streams * sizeof(int));
    int *counter = (int *)malloc(num_streams * sizeof(int));        // total recv packets in each stream
    int *bytes_received = (int *)malloc(num_streams * sizeof(int)); // total amount of data recv
    /* this delay needs sync clock, but not needed for now*/
    uint64_t *acc_delay = (uint64_t *)malloc(num_streams * sizeof(uint64_t)); // total delay accumulated for each stream
    double *throughputs = (double *)malloc(num_streams * sizeof(double));
    double *prev_transit = (double *)malloc(num_streams * sizeof(double));
    double *jitters = (double *)malloc(num_streams * sizeof(double));
    for (int i = 0; i < num_streams; i++)
    {
        first_packet[i] = 0;
        packets_count[i] = 0;
        loss_num[i] = 0;
        out_of_order_pkt_num[i] = 0;
        counter[i] = 0;
        bytes_received[i] = 0;
        acc_delay[i] = 0;
        throughputs[i] = 0;
        prev_transit[i] = 0;
        jitters[i] = 0;
    }
    if (stream_id == 1) // set timeout for the recv socket
    {
        struct timeval timeout;
        timeout.tv_sec = 5; // in default 5s
        timeout.tv_usec = 0;
        setsockopt(sockSer, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
    }
    printf("Server starts, packet size %d\n", RECV_UNIT);
    while (1)
    {
        int r = recvfrom(sockSer, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&addrCli, (socklen_t *)&totalen);
        if (r < 0)
        {
            if (errno == EAGAIN) // if socket waiting is timeout
            {
                break;
            }
            perror("recv from error:");
            exit(1);
        }
        memcpy(&stream_id, recvbuf, sizeof(stream_id));
        memcpy(&sec, recvbuf + 4, sizeof(sec));
        memcpy(&usec, recvbuf + 8, sizeof(usec));
        memcpy(&pcount, recvbuf + 12, sizeof(pcount));
        memcpy(&signal, recvbuf + 16, sizeof(signal));
        stream_id = ntohl(stream_id);
        sec = ntohl(sec);
        usec = ntohl(usec);
        pcount = ntohl(pcount);
        signal = ntohl(signal);
        bytes_received[stream_id - 1] += r;
        counter[stream_id - 1]++;
        if (bytes_received[stream_id - 1] == 0) // start of the each stream
        {
            printf("stream id %d\n", stream_id);
            iperf_time_now(&first_arr_time);
            first_packet[stream_id - 1] = 1;
        }
        if (signal && !throughputs[stream_id] - 1)
        { // if the signal pkts are lost, the corresponding throughputs[i] would be 0
            iperf_time_now(&arrival_time);
            iperf_time_diff(&arrival_time, &first_arr_time, &temp_time);
            double duration = iperf_time_in_secs(&temp_time);
            throughputs[stream_id - 1] = bytes_received[stream_id - 1] / (duration * 1024 * 1024); //xx Mb/s
        }
        sent_time.secs = sec;
        sent_time.usecs = usec;
        if (pcount >= packets_count[stream_id - 1] + 1)
        {

            /* Forward, but is there a gap in sequence numbers? */
            if (pcount > packets_count[stream_id - 1] + 1)
            {
                /* There's a gap so count that as a loss. */
                loss_num[stream_id - 1] += (pcount - 1) - packets_count[stream_id - 1];
            }
            /* Update the highest sequence number seen so far. */
            packets_count[stream_id - 1] = pcount;
        }
        else
        {
            /* 
	     * Sequence number went backward (or was stationary?!?).
	     * This counts as an out-of-order packet.
	     */

            out_of_order_pkt_num[stream_id - 1]++;

            /*
	     * If we have lost packets, then the fact that we are now
	     * seeing an out-of-order packet offsets a prior sequence
	     * number gap that was counted as a loss.  So we can take
	     * away a loss.
	     */
            if (loss_num[stream_id - 1] > 0)
            {
                loss_num[stream_id - 1]--;
            }
        }

            /*
        * jitter measurement
        *
        * This computation is based on RFC 1889 (specifically
        * sections 6.3.1 and A.8).
        *
        * Note that synchronized clocks are not required since
        * the source packet delta times are known.  Also this
        * computation does not require knowing the round-trip
        * time.
        */
        iperf_time_now(&arrival_time);
        iperf_time_diff(&arrival_time, &sent_time, &temp_time); // needs sync between client and server
        acc_delay[stream_id - 1] += iperf_time_in_usecs(&temp_time);
        transit = iperf_time_in_secs(&temp_time);

        /* Hack to handle the first packet by initializing prev_transit. */
        if (first_packet[stream_id - 1])
            prev_transit[stream_id - 1] = transit;

        d = transit - prev_transit[stream_id - 1];
        if (d < 0)
            d = -d;
        prev_transit[stream_id - 1] = transit;
        jitters[stream_id - 1] += (d - jitters[stream_id - 1]) / 16.0;
    }
    close(sockSer);

    // get results
    double loss_rate, delay_per_stream;
    int total_loss = 0;
    int total_packets = 0;
    double total_throughputs = 0;
    double total_delay = 0;
    double total_jitter = 0;
    int valid_streams = 0;
    FILE *fp;
    if (proto_id == 1)
        fp = fopen("udp1.txt", "a");
    else if (proto_id == 2)
        fp = fopen("udplite1.txt", "a");
    else if (proto_id == 3)
        fp = fopen("udpfec.txt", "a");
    fprintf(fp, "Stream number | highest number | number pkt | loss pkt | ooo pkt | loss rate | throughput | delay | jitter |\n");
    for (int i = 0; i < num_streams; i++)
    {
        loss_rate = (double)loss_num[i] / packets_count[i] * 100;
        delay_per_stream = acc_delay[i] / (counter[i] * 1000); // ms
        fprintf(fp, "%14.d %16.d %12.d %10.d %9.d %10.3lf%% %12.3lfM/s %7.3lfms %8.3lfms\n",
                i + 1, packets_count[i], counter[i], loss_num[i], out_of_order_pkt_num[i], loss_rate, throughputs[i], delay_per_stream, jitters[i] * 1000);
        // printf("Stream number | highest number | number pkt | loss pkt | ooo pkt | loss rate\n");
        // printf("%14.d %16.d %12.d %10.d %9.d %9.3lf%%\n", i + 1, packets_count[i], counter[i], loss_num[i], out_of_order_pkt_num[i], loss_rate);
        total_loss += loss_num[i];
        total_packets += packets_count[i];
        if (throughputs[i])
        {
            total_throughputs += throughputs[i];
            valid_streams++;
        }
        total_delay += delay_per_stream;
        total_jitter += jitters[i];
    }
    double aver_loss_rate = (double)total_loss / total_packets * 100;
    double aver_throuput = total_throughputs / valid_streams;
    double aver_delay = total_delay / num_streams;
    double aver_jitter = total_jitter / num_streams;
    fprintf(fp, "Server: The average loss rate is %.4lf%%\n", aver_loss_rate);
    fprintf(fp, "Server: The average throughput is %.4lf%%\n", aver_throuput);
    fprintf(fp, "Server: The average delay is %.4lf%%\n", aver_delay);
    fprintf(fp, "Server: The average jitter is %.4lf%%\n", aver_jitter);
    fprintf(fp, "----------\n");
    fclose(fp);
    // printf("Server: The average loss rate is %.4lf%%\n", aver_loss_rate);

    //cleanup
    free(first_packet);
    free(packets_count);
    free(loss_num);
    free(out_of_order_pkt_num);
    free(counter);
    free(bytes_received);
    free(acc_delay);
    free(throughputs);
    free(prev_transit);
    free(jitters);

    return 0;
}