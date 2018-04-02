#ifndef SERVER_APP_H
#define SERVER_APP_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>
#include "APP_PROTOCOL.h"

#define MYPORT 3490    // the port users will be connecting to
#define BACKLOG 10     // how many pending connections queue will hold

class app {
public:
//    void app();
    int start(std::string &msg);
    void stop();
    static void sigchld_handler(int s) { while(wait(NULL) > 0); }

    enum APP_RESPONSE {
        OK = 0,
        SOCKET = 1,
        SETSOCKOPT = 2,
        BIND = 3,
        LISTEN = 4,
        SIGACTION = 5,
        RECV = 6,
        SEND = 7
    };

private:
    int sockfd, new_fd;
    struct sockaddr_in their_addr; // connector's address information
    int add_user();
};


#endif //SERVER_APP_H
