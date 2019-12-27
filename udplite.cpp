#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>  
#include <netdb.h>  
#include <fcntl.h>  
#include <unistd.h>  
#include <stdlib.h>
#include <sys/stat.h>  
#include <sys/types.h>  
#include <arpa/inet.h>
#include <time.h>
#include <iostream>
#include <string.h>
#include <stdint.h>
#include <csignal>
#include <iostream>
#include <assert.h>
#include <math.h>

using namespace std;

/////////////////////////////////////////////////////////
const float CPU_freq = 2.904004;
//const float CPU_freq = 2.194844;
// const float CPU_freq = 2.808002;

typedef struct timestamp {
    uint32_t coreId;
    uint64_t value;
} timestamp_t;

inline void read_timestamp_counter(timestamp_t * t) {
    uint64_t highTick, lowTick;
    // asm is used to write assembly language in c++
    asm volatile ("rdtscp" : "=d"(highTick), "=a"(lowTick), "=c"(t->coreId));
    t->value = highTick << 32 | lowTick;
}

inline uint64_t
diff_timestamps(const timestamp_t * before,
                const timestamp_t * after) {
    assert(before->coreId == after->coreId);
    return after->value - before->value;
}

inline uint64_t
cycle_since_timestamp(const timestamp_t * previous) {
    timestamp_t now;
    read_timestamp_counter(&now);
    return diff_timestamps(previous, &now);
}

uint64_t get_time(){
    struct timespec tv;
    clock_gettime(CLOCK_REALTIME,&tv);

    uint64_t clock = (tv.tv_sec % 1000) * 1e9 + tv.tv_nsec;

    return clock;
}
/////////////////////////////////////////////////////////

typedef unsigned int SOCKET;
#define INVALID_SOCKET  -1  
#define SOCKET_ERROR    -1  

#define BUFFER_SZ 1200
#define SEND_BUF_LEN 512

int server_port = 33456;
int client_port = 33457;

const char* ServerIP = "127.0.0.1";

int Send_num = 300000;

const int RTT_LEN = 1000000;
uint64_t *rtt = new uint64_t[RTT_LEN];

int mode = -1;

// void sig_handler(int sig) {
//     if ( sig == SIGINT){
//         // sender
//         if(mode == 0){

//         }
//         // receiver
//         else{

//         }
//         delete []rtt;
//         close(m_sock);
//     }
//     exit(0);
// }


int main(int argc, char *argv[]){
    if(argc != 2){
        std::cout << "Usage: APP NAME, MODE(0:Server, 1:Client)" << std::endl;
    }
    mode = atoi(argv[1]);

    /*Server: pong*/
    /*Client: ping*/

    //注册中断信号
    // signal( SIGINT, sig_handler );

    ///Server
    if(mode == 0){
        SOCKET m_sock;
        char buffer[BUFFER_SZ];
        //Create a socket
        if((m_sock = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDPLITE )) == INVALID_SOCKET)
        {
            return 0;
        }
        sockaddr_in my_addr;
        sockaddr_in remote_addr; //客户端网络地址结构体  
        memset(&my_addr,0,sizeof(sockaddr_in));
        //Prepare the sockaddr_in structure
        my_addr.sin_family = AF_INET;
        my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        my_addr.sin_port = htons( server_port );

        //Bind
        if( bind(m_sock ,(sockaddr *)&my_addr , sizeof(sockaddr)) == SOCKET_ERROR)
        {
            printf("Socket bind failed\n");
            close(m_sock);
            return 0;
        }

        int sin_size=sizeof(struct sockaddr_in);  

        cout << "waiting for packets..." << endl;
        for(int i=0; i < Send_num; i++)
        {
            // do_pong
            int nRecEcho = recvfrom(m_sock, (char*)buffer, BUFFER_SZ , 0, (struct sockaddr *)&remote_addr, (socklen_t *)&sin_size);  
            if (nRecEcho < 0)  
            {  
                printf("[SERVER]recv error/n");  
                break;  
            }
            else if (nRecEcho != 512){
                printf("invalid packet received!\n");
            }
            int len = sendto(m_sock, buffer, nRecEcho, 0, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));
            if(len == -1){
                std::cout << "[SERVER]send failed!" << std::endl;
            }
        }
        close(m_sock);
        cout << "complete!" << endl;
    }
    ///Client
    else{
        SOCKET m_sock;
        char buffer[SEND_BUF_LEN];

        if((m_sock = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDPLITE )) == INVALID_SOCKET)
        {
            return 0;
        }
        sockaddr_in remote_addr;
        memset(&remote_addr,0,sizeof(sockaddr_in));
        //Prepare the sockaddr_in structure
        remote_addr.sin_family = AF_INET;
        remote_addr.sin_addr.s_addr = inet_addr(ServerIP);//服务器IP地址  
        remote_addr.sin_port = htons( server_port );


        sockaddr_in my_addr;
        memset(&my_addr,0,sizeof(sockaddr_in));
        //Prepare the sockaddr_in structure
        my_addr.sin_family = AF_INET;
        my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        my_addr.sin_port = htons( client_port );

        //Bind
        if( bind(m_sock ,(sockaddr *)&my_addr , sizeof(sockaddr)) == SOCKET_ERROR)
        {
            printf("Socket bind failed\n");
            close(m_sock);
            return 0;
        }

        memset(buffer, 36, SEND_BUF_LEN);

        int sin_size=sizeof(struct sockaddr_in);  

        // do_ping
        for(int i=0; i < Send_num; i++){
            timestamp_t start;
            read_timestamp_counter(&start);

            buffer[0] = 0xff;
            buffer[1] = 0xee;
            buffer[2] = 0x00;
            buffer[3] = 0x02;//little endien
            memcpy(buffer + 110, &i, sizeof(i));
            memcpy(buffer + 50, &(start), sizeof(start));
            // int len = sendto(m_sock, buffer, sizeof(i), 0, NULL, 0);
            // std::cout << "sending: " << i << std::endl;
            int len = sendto(m_sock, buffer, SEND_BUF_LEN, 0, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));
            if(len == -1){
                std::cout << "socket send failed!" << std::endl;
            }

            int nRecEcho = recvfrom(m_sock, (char*)buffer, BUFFER_SZ , 0, (struct sockaddr *)&remote_addr, (socklen_t *)&sin_size);  
            if (nRecEcho < 0)  
            {  
                printf("[CLIENT]recv error/n");  
                break;  
            }
            else if (nRecEcho != 512){
                printf("[CLIENT]invalid packet received!\n");
            }

            uint64_t latency = cycle_since_timestamp(&start);
            rtt[i] = latency / CPU_freq / 2;
        }
        close(m_sock);

        uint64_t min = 99999, max = 0, sum = 0;
        // find the maximum latency and minimum latency
        for(int i=0; i < Send_num; i++){
            // fout << rtt[i] << endl;
            if (rtt[i] < min){
                min = rtt[i];
            }
            if (rtt[i] > max){
                max = rtt[i];
            }
            sum += rtt[i];
        }
        double avg = (double)sum / Send_num;
        double std = 0.0;
        double tmp = 0.0;    
        for(int i=0; i < Send_num; i++){
            tmp += (rtt[i] - avg) * (rtt[i] - avg);
        }
        double variance = tmp/Send_num;
        std = sqrt(tmp/Send_num);
        double tmpJitter = 0.0;
        for(int i = 0; i < Send_num; i++){
            tmpJitter += abs(rtt[i] - avg);
        }
        double jitter = tmpJitter/Send_num;

        cout << "num: " << Send_num << endl;
        cout << "avg: " << avg << endl;
        cout << "min: " << min << endl;
        cout << "max: " << max << endl;
        cout << "jitter:" << jitter << endl;
        cout << "std: " << std << endl;
        cout << "variance: " << variance << endl;
        cout << "sum: " << sum << endl;
    }

    return 0;
}