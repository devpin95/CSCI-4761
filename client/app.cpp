//
// Created by devpin on 3/31/2018.
//

#include "app.h"

static const std::string C_1 = "A";
static const std::string C_2 = "B";
static const std::string C_3 = "C";

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
    if ((he=gethostbyname(argv[1])) == NULL) {  // get the host info
        return APP_RESPONSE::GETHOSTBYNAME;
//        perror("gethostbyname");
//        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return APP_RESPONSE::SOCKET;
//        perror("socket");
//        exit(1);
    }

    their_addr.sin_family = AF_INET;    // host byte order
    their_addr.sin_port = htons(PORT);  // short, network byte order
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct

    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        return APP_RESPONSE::CONNECT;
//        perror("connect");
//        exit(1);
    }

    printf("connection has been established with server. Type any message for server\n");

    for(;;) {
        int choice;
        const char* control;
        bzero(stringbuf, sizeof(stringbuf));
        std::cout << "1. TRUE" << std::endl << "2. aaaaAAAAAAAHHHHHHHHH" << std::endl << "3. Message" << std::endl
                  << "4. Exit" << std::endl << "> ";
        std::cin >> choice;
        if (choice == 1) {
            control = C_1.c_str();
        }
        else if ( choice == 2 ) {
            control = C_2.c_str();
        }
        else if (choice == 3) {
            if ((numbytes=send(sockfd, "C", sizeof(char), 0)) == -1) {
                perror("send");
                close(sockfd);
                exit(1);
            }
            std::cout << "Message: ";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
            bzero(stringbuf, sizeof(stringbuf));
            gets(stringbuf);

            if ((numbytes=send(sockfd, stringbuf, sizeof(char)*strlen(stringbuf), 0)) == -1) {
                perror("send");
                close(sockfd);
                exit(1);
            }
            continue;
        } else if ( choice == 4 ) {
            break;
        }

        if ((numbytes=send(sockfd, control, sizeof(char) * strlen(control), 0)) == -1) {
            perror("send");
            close(sockfd);
            exit(1);

//            stringbuf[numbytes]='\0';
//            printf("Sent: %s\n",controlbuf);

//            if ((numbytes=recv(sockfd, buf, 127, 0)) == -1) {
//                perror("recv");
//                exit(1);
//            }
//
//            buf[numbytes] = '\0';
//
//            printf("Received: %s\n",buf);
        }
    }
    close(sockfd);
}

void app::stop() {
    close(sockfd);
}