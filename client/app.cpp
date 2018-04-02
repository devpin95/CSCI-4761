//
// Created by devpin on 3/31/2018.
//

#include "app.h"

int app::start(std::string &msg, int argc, char *argv[]) {
    int numbytes;
    char buf[MAXDATASIZE];
    char stringbuf[MAXDATASIZE];
    char controlbuf[MAXCONTROLSIZE];
    struct hostent *he;
    struct sockaddr_in their_addr; // connector's address information

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    //ucdencsecnode05.ucdenver.pvt
    //DESKTOP-IHAFUS8
    if ((he=gethostbyname(argv[1])) == NULL) {  // get the host info
        return APP_RESPONSE::GETHOSTBYNAME;

    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return APP_RESPONSE::SOCKET;
    }

    their_addr.sin_family = AF_INET;    // host byte order
    their_addr.sin_port = htons(PORT);  // short, network byte order
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct

    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        return APP_RESPONSE::CONNECT;
    }

    printf("connection has been established with server. Type any message for server\n");

    for(;;) {
        int choice;
        std::cout << "1. Log In" << std::endl << "2. Create Account" << std::endl
                  << "3. Exit" << std::endl << "> ";

            std::cin >> choice;

        if (choice == 1) {
           // control = C_LOGIN.c_str();
            std::cout << "TODO" << std::endl;
        }
        else if ( choice == 2 ) {
            //control = C_ADD_USER.c_str();
            int resp = add_user();
            if (resp == -1) {
                return resp;
            }
        }
        else if ( choice == 3 ) {
            return APP_RESPONSE::OK;
        }

//        if ((numbytes=send(sockfd, control, sizeof(char) * strlen(control), 0)) == -1) {
//            perror("send");
//            close(sockfd);
//            exit(1);
//        }
    }
    close(sockfd);
}

void app::stop() {
    close(sockfd);
}

int app::login() {

}

int app::add_user() {
    const char* control;
    char unamebuf[MAX_USERNAME_LENGTH] = {'\0', };
    char passbuf[MAX_PASSWORD_LENGTH] = {'\0', };
    control = C_LOGIN.c_str();

    //send control byte
    if ((send(sockfd, control, /* sizeof(char) * strlen(control) */ MAXCONTROLSIZE, 0)) == -1) {
        return APP_RESPONSE::SEND;
    }


    //std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    while(true) {
        bzero(unamebuf, sizeof(unamebuf));

        std::cout << "User name (up to 20 characters): ";
        gets(unamebuf);
        unamebuf[sizeof(unamebuf)] = '\0';

        if ( ERRCHECKER::USERNAME(unamebuf) ) {
            break;
        } else {
            std::cout << unamebuf << " is not a valid username" << std::endl;
        }
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    while(true) {
        bzero(passbuf, sizeof(passbuf));

        std::cout << "Password (8-16 characters): ";
        gets(passbuf);

        if ( ERRCHECKER::PASSWORD(passbuf) ) {
            break;
        } else {
            std::cout << "Password must be 8-16 characters" << std::endl;
        }
    }

    if ((send(sockfd, unamebuf, sizeof(char)*strlen(unamebuf), 0)) == -1) {
        return APP_RESPONSE::SEND;
    }

    if ((send(sockfd, passbuf, sizeof(char)*strlen(passbuf), 0)) == -1) {
        return APP_RESPONSE::SEND;
    }
}