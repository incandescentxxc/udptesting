#include <stdio.h>

struct stream
{
    int id;
    double loss;
};

/*
* Control parameters
*/
struct test
{
    char role; /*'c'lient, 's'erver*/
    int packets_num_to_sent;
    char *bind_address; /* first -B option */
    int server_port;
    int num_stream;    
};
