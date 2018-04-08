
/*
** client.c -- a stream socket client demo
*/

#include "app.h"

int main(int argc, char *argv[])
{
    app clientapp;
    std::string clientmsg;
    int resp = clientapp.start(clientmsg, argc, argv);
    switch (resp) {
        case 0: std::cout << "Disconnected";
            break;
        case 1: std::cout << clientmsg << " " ; perror("gethostbyname");;
            break;
        case 2: std::cout << clientmsg << " "; perror("socket");
            break;
        case 3: std::cout << clientmsg << " "; perror("connect");
            break;
        case 4: std::cout << clientmsg << " "; perror("listen");
            break;
        case 5: std::cout << clientmsg << " "; perror("send");
            break;
        case 6: std::cout << clientmsg << " "; perror("recv");
            break;
        case 7: fprintf(stderr,"usage: client hostname\n");
            break;
        default: break;
    }
    clientapp.stop();
    return 0;
}

