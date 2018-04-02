#ifndef CLIENT_APP_H
#define CLIENT_APP_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include <iostream>
#include <climits>
#include <limits>
#include "APP_PROTOCOL.h"

#define PORT 3490 // the port client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at once
#define MAXCONTROLSIZE 1

class app {
public:
    int start(std::string &msg, int argc, char *argv[]);
    void stop();

    enum APP_RESPONSE {
        OK = 0,
        GETHOSTBYNAME = 1,
        SOCKET = 2,
        CONNECT = 3,
        LISTEN = 4,
        SEND = 5,
        RECV = 6
    };

private:
    int login();
    int add_user();
    int sockfd;
};


#endif //CLIENT_APP_H
