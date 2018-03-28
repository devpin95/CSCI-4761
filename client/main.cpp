
/*
** client.c -- a stream socket client demo
*/

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

#define PORT 3490 // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once
#define MAXCONTROLSIZE 1

static const std::string C_1 = "A";
static const std::string C_2 = "B";
static const std::string C_3 = "C";

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    char stringbuf[MAXDATASIZE];
    char controlbuf[MAXCONTROLSIZE];
    struct hostent *he;
    struct sockaddr_in their_addr; // connector's address information
//    S TEST1, TEST2;
//    TEST1.s = 'A';
//    TEST2.s = 'B';

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    if ((he=gethostbyname(argv[1])) == NULL) {  // get the host info 
        perror("gethostbyname");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    their_addr.sin_family = AF_INET;    // host byte order 
    their_addr.sin_port = htons(PORT);  // short, network byte order 
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct 

    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(1);
    }

    printf("connection has been established with server. Type any message for server\n");

    for(;;) {
        int choice;
        const char* control;
        std::cout << "1. JILL" << std::endl << "2. aaaaAAAAAAAHHHHHHHHH" << std::endl << "3. Message" << std::endl
                  << "4. Exit" << "> ";
        std::cin >> choice;
        if (choice == 1) {
            control = C_1.c_str();
        } else if ( choice == 2 ) {
            control = C_2.c_str();
        } else if (choice == 3) {
            control = C_3.c_str();
            if ((numbytes=send(sockfd, control, sizeof(control), 0)) == -1) {
                perror("send");
                close(sockfd);
                exit(1);
            }
            std::cout << "Message: ";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
            gets(stringbuf);
            numbytes=sizeof(stringbuf);
            stringbuf[numbytes]='\0';
            control = stringbuf;
        } else if ( choice == 4 ) {
            break;
        }
        if ((numbytes=send(sockfd, control, sizeof(control), 0)) == -1) {
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

    return 0;
}

