
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

#define PORT 3490 // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    char sendbuf[MAXDATASIZE];
    struct hostent *he;
    struct sockaddr_in their_addr; // connector's address information 

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
    	gets(sendbuf);
    	numbytes=sizeof(sendbuf);
    	sendbuf[numbytes]='\0';
	if (numbytes == 0 || strncmp(sendbuf,"bye",3)==0) {
	    printf("Bye\n");
	    break;
	}
    	else {
		if ((numbytes=send(sockfd, sendbuf, sizeof(sendbuf), 0)) == -1) {
                perror("send");
                close(sockfd);
		exit(1);
    		}

		sendbuf[numbytes]='\0';
		printf("Sent: %s\n",sendbuf);

		if ((numbytes=recv(sockfd, buf, 127, 0)) == -1) {
			perror("recv");
			exit(1);
		}

		buf[numbytes] = '\0';
	
    		printf("Received: %s\n",buf);
        }
    }
    close(sockfd);

    return 0;
}

