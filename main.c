#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "api.h"
#define DEST_PORT 8000

static int run(struct test *newtest);

int main(int argc, char *argv[])
{
    struct test *newtest = (struct test *)malloc(sizeof(struct test));
    newtest->role = argv[1];
    newtest->packets_num_to_sent = atoi(argv[2]);
    newtest->num_stream = 5;
    newtest->server_port = 8000;
    newtest->bind_address = "172.16.33.29";

    if ((run(newtest)) < 0)
    {
        perror("server/client running fails:\n");
    }
}

static int run(struct test *newtest)
{
    switch (newtest->role)
    {
    case 'c':
        int rc;
    }
}