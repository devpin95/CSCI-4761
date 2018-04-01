
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
        case 0: std::cout << clientmsg << " " << "Disconnected";
        case 1: std::cout << clientmsg << " " ; perror("gethostbyname");;
            break;
        case 2: std::cout << clientmsg << " "; perror("socket");
            break;
        case 3: std::cout << clientmsg << " "; perror("connect");
            break;
        case 4: std::cout << clientmsg << " "; perror("listen");
            break;
        case 5: break;
        default: break;
    }
    return 0;
}

