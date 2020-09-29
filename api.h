#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include "types.h"


/* Server routines. */
int run_server(struct test *);
int server_listen(struct test *);
int accept(struct test *);

/* Client routines. */
int run_client(struct test *);
struct stream* create_streams(int num);
int connect(struct test*);
int handle_message_client(struct test*);
int client_end(struct test*);


   