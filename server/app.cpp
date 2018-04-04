#include "app.h"
//void app::app( ) {
//    // constructor
//}

int app::start( std::string &msg ) {
//    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct sockaddr_in my_addr;    // my address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char *recvbuf;
    char *caddr;
    int  numbytes;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return APP_RESPONSE::SOCKET;
    }

    printf("SOCK_FD=%d\n",sockfd);
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        return APP_RESPONSE::SETSOCKOPT;
    }

    my_addr.sin_family = AF_INET;         // host byte order
    my_addr.sin_port = htons(MYPORT);     // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        return APP_RESPONSE::BIND;
    }

    if (listen(sockfd, BACKLOG) == -1) {
        int e = errno;
        std::cout << "errno -> " << e;
        return APP_RESPONSE::LISTEN;
    }

    sa.sa_handler = app::sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        return APP_RESPONSE::SIGACTION;
    }

    while(1) {  // main accept() loop
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
            perror("accept");
            continue;
        }
        printf("server: got connection from %s\n",(char *) inet_ntoa(their_addr.sin_addr));
        msg = inet_ntoa(their_addr.sin_addr);

        //if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
            recvbuf = (char *) calloc(128,sizeof(char));

            for(;;) {
                bzero(recvbuf, sizeof(recvbuf));
                numbytes=recv(new_fd, recvbuf, MAXCONTROLSIZE, 0);

                if (numbytes == -1) {
                    int e = errno;
                    std::cout << "errno -> " << e;
                    fprintf(stderr, "(1) numbytes = %d\n", numbytes);
                    return APP_RESPONSE::RECV;
                }
                else if ( numbytes==0 ) {
                    break;
                }

                printf("numbytes: %d\n", numbytes);

                //..... Do stuff
                if ( atoi( &recvbuf[0] ) == C_LOGIN_BIN )
                {
                    fprintf(stderr, "Calling addUser()\n" );
                    int resp = addUser();
                    if ( resp != -1 ) {
                        return resp;
                    }
                    //printf("Received from %s: %s\n",inet_ntoa(their_addr.sin_addr), " THE PROPHECY IS TRUE\n");
                }

                else if ( recvbuf[0] == 'B' )
                {
                    //printf("Received from %s: %s\n",inet_ntoa(their_addr.sin_addr), " It is Wednesday, my dudes. aaaaaAAAAAAHHHHHHHHHH\n");
                }

                else if ( recvbuf[0] == 'C')
                {
                    printf("LISTENING FOR MESSAGE\n");
                    numbytes = recv(new_fd, recvbuf, 127, 0);
                    if (numbytes < 0) {
                        int e = errno;
                        std::cout << "errno -> " << e;
                    }
                    printf("Received from %s: %s\n", inet_ntoa(their_addr.sin_addr), recvbuf);
                }
            }

            free(recvbuf);
            return APP_RESPONSE::OK;
        //}
        close(new_fd);  // parent doesn't need this
    }
}

void app::stop() {
    close(new_fd);
}

int app::addUser() {
    // response values
    // 0000 0001 (1) – OK
    // 0000 0010 (2) - ERR
    // 0000 0010 (3) – Username already used
    // 0000 0011 (4) – Malformed Data

    const int OK = 0b0001;
    const int ERR = 0b0010;
    const int UNAME_USED = 0b0011;
    const int MAL_DATA = 0b0100;

    char unamebuf[MAX_USERNAME_LENGTH] = { 0, };
    int numbytes = recv( new_fd, unamebuf, MAX_USERNAME_LENGTH, 0 );
    fprintf(stderr, "(2) numbytes = %d\n", numbytes);
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;

    printf("Received USERNAME from %s: %s\n",inet_ntoa(their_addr.sin_addr), unamebuf );

    char passbuf[MAX_USERNAME_LENGTH] = { '\0', };
    numbytes = recv( new_fd, passbuf, MAX_USERNAME_LENGTH, 0 );

    if ( numbytes == -1 ) {
        int e = errno;
        std::cout << "errno -> " << e;
        return APP_RESPONSE::RECV;
    }
    else if ( numbytes == 0 ) {
        int e = errno;
        std::cout << "errno -> " << e;
        return APP_RESPONSE::OK;
    }

    printf( "Received PASSWORD from %s: %s\n",inet_ntoa( their_addr.sin_addr ), passbuf );
    std::cout << "SENDING LOGIN RESPONSE... ";

    const char* control;

    // only check if the username exists if the name is valid
    control = std::to_string( OK ).c_str( );
    if ( ERRCHECKER::USERNAME( unamebuf ) && ERRCHECKER::PASSWORD( passbuf ) )
    {
        int ex = db.addUser( unamebuf, passbuf );
        int e = errno;
        if ( ex == -1 ) {
            if ( e == EEXIST ) {
                control = std::to_string( UNAME_USED ).c_str();
            }
            else {
                control = std::to_string( ERR ).c_str();
            }
        }
    }
    else
    {
        // Bad data
        control = std::to_string( MAL_DATA ).c_str( );
    }

    std::cout << control << std::endl;
    if ( ( send( new_fd, control, MAXCONTROLSIZE, 0 ) ) == -1 )
    {
        int e = errno;
        std::cout << "(-1) errno -> " << e;
        return APP_RESPONSE::SEND;
    }

    return -1;
}

int app::checkIfUserExists(const std::string &uname) {

}