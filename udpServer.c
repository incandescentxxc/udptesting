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
    int num_streams = 10; // in default receive 10 streams
    if (argv[1])
    {
        num_streams = atoi(argv[1]);
    }
    int df_duration = 5; // in default 3s
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
    uint32_t sec, usec, pcount;
    struct iperf_time sent_time, arrival_time, temp_time, first_arr_time;
    iperf_time_now(&first_arr_time);
    iperf_time_now(&arrival_time);
    iperf_time_diff(&arrival_time, &first_arr_time, &temp_time);
    int stream_id = 1;
    int *first_packet = (int *)malloc(num_streams * sizeof(int));
    int *packets_count = (int *)malloc(num_streams * sizeof(int)); // highest number received so far
    int *loss_num = (int *)malloc(num_streams * sizeof(int));
    int *out_of_order_pkt_num = (int *)malloc(num_streams * sizeof(int));
    int *counter = (int *)malloc(num_streams * sizeof(int));
    int *bytes_received = (int *)malloc(num_streams * sizeof(int));
    double *acc_delay = (double *)malloc(num_streams * sizeof(double));
    for (int i = 0; i < num_streams; i++)
    {
        first_packet[i] = 0;
        packets_count[i] = 0;
        loss_num[i] = 0;
        out_of_order_pkt_num[i] = 0;
        counter[i] = 0;
        bytes_received[i] = 0;
        acc_delay[i] = 0;
    }
    printf("Server starts, packet size %d\n", RECV_UNIT);
    while (1)
    {
        int r = recvfrom(sockSer, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&addrCli, (socklen_t *)&totalen);
        if (r < 0)
        {
            perror("recv from error:");
            exit(1);
        }
        memcpy(&stream_id, recvbuf, sizeof(stream_id));
        stream_id = ntohl(stream_id);
        if (stream_id <= num_streams)
        {
            memcpy(&sec, recvbuf + 4, sizeof(sec));
            memcpy(&usec, recvbuf + 8, sizeof(usec));
            memcpy(&pcount, recvbuf + 12, sizeof(pcount));
            if (bytes_received[stream_id - 1] == 0)
            {
                iperf_time_now(&first_arr_time);
                first_packet[stream_id - 1] = 1;
            }
            bytes_received[stream_id - 1] += r;
            sec = ntohl(sec);
            usec = ntohl(usec);
            pcount = ntohl(pcount);
            sent_time.secs = sec;
            sent_time.usecs = usec;
            iperf_time_now(&arrival_time);
            iperf_time_diff(&arrival_time, &sent_time, &temp_time);
            acc_delay[stream_id - 1] += iperf_time_in_usecs(&temp_time);
            counter[stream_id - 1]++;
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
        }
        else
        {
            break; // consider the extra signal stream
        }
    }
    close(sockSer);

    // get results
    double loss_rate;
    int total_loss = 0;
    int total_packets = 0;
    for (int i = 0; i < num_streams; i++)
    {
        loss_rate = (double)loss_num[i] / packets_count[i];
        printf("Stream number | highest number | number pkt | loss pkt | ooo pkt | loss rate\n");
        printf("%14.d %16.d %12.d %10.d %9.d %10.3lf\n", i + 1, packets_count[i], counter[i], loss_num[i], out_of_order_pkt_num[i], loss_rate);
        total_loss += loss_num[i];
        total_packets += packets_count[i];
    }
    double aver_loss_rate = (double)total_loss / total_packets;
    // FILE *fp;
    // fp = fopen("test.txt", "a");
    // fprintf(fp, "Average loss rate is %.4lf%%\n", aver_loss_rate);
    // fclose(fp);
    printf("Server: The average loss rate is %.4lf%%\n", aver_loss_rate);

    //cleanup
    free(first_packet);
    free(packets_count);
    free(loss_num);
    free(out_of_order_pkt_num);
    free(counter);
    free(bytes_received);
    free(acc_delay);

    return 0;
}