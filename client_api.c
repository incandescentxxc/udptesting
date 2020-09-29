#include "api.h"

int connect(struct test* newtest){
    int sockCli = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockCli == -1){
        perror("Client socket create fail: ");
    }
    struct sockaddr_in addrCli;
    memset(&addrCli, 0, sizeof(addrCli));
    addrCli.sin_family = AF_INET;
    addrCli.sin_port = htons(newtest->server_port);
    addrCli.sin_addr.s_addr = inet_addr(newtest->bind_address);
}

struct stream* create_streams(int num){
    struct stream* streams = (struct stream*)malloc(num * sizeof(struct stream));
    for(int i = 0; i < num; i++){
        streams[i].id = i+1;
        streams[i].loss = 0;
    }
    return streams;
}

int run_client(struct test* newtest){
    int num_packets_to_send = newtest->packets_num_to_sent;
    int num_streams = newtest->num_stream;
    struct timeval t1, t2;
    double elapsedTime;
    gettimeofday(&t1, NULL);
    struct stream* streams = create_streams(num_streams);
    for(int i = 0; i < num_streams; i++){
        int packetsent = 0;
        while(packetsent < num_packets_to_send){
            
        }
    }

}
