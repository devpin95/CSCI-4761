#include <iostream>
#include "app.h"

int main(void) {
    app sapp;
    std::string servermsg;
    int resp = sapp.start(servermsg);
    switch (resp) {
        case 0: std::cout << servermsg << " " << "Disconnected";
        case 1: std::cout << servermsg << " " ; perror("socket");
            break;
        case 2: std::cout << servermsg << " "; perror("setsockopt");
            break;
        case 3: std::cout << servermsg << " "; perror("bind");
            break;
        case 4: std::cout << servermsg << " "; perror("listen");
            break;
        case 5: break;
        case 6 : std::cout << servermsg << " "; perror("recv");
        default: break;
    }
    sapp.stop();
    return 0;
}