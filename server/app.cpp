#include <assert.h>
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
                int cont;
                numbytes=recv(new_fd, &cont, sizeof(int), 0);

                if (numbytes == -1) {
                    int e = errno;
                    std::cout << "errno -> " << e << std::endl;
                    fprintf(stderr, "(1) numbytes = %d\n", numbytes);
                    fflush(stderr);
                    return APP_RESPONSE::RECV;
                }
                else if ( numbytes==0 ) {
                    break;
                }

                //printf("numbytes: %d\n", numbytes);

                //..... Do stuff
                if ( cont == C_DISCONN_BIN )
                {
                    fprintf(stderr, "User disconnecting\n" );
                    return APP_RESPONSE::OK;
                }
                else if ( cont == C_ADD_USER_BIN ) {
                    fprintf(stderr, "Calling addUser()\n" );
                    fflush(stderr);
                    int resp = addUser();
                    if ( resp != -1 ) {
                        return resp;
                    }
                }
                else if ( cont == C_LOGIN_BIN )
                {
                    fprintf(stderr, "Calling login()\n" );
                    int resp = login();
                    if ( resp != -1 ) {
                        return resp;
                    }
                }
                else if ( cont == C_GET_USER_APPTS_BIN )
                {
                    fprintf(stderr, "Calling getAppts()\n" );
                    fflush(stderr);
                    int resp = getAppts();
                    if ( resp != -1 ) {
                        return resp;
                    }
                }
                else if ( cont == C_ADD_APPT_BIN )
                {
                    fprintf(stderr, "Calling addAppt()\n" );
                    fflush(stderr);
                    int resp = addAppt();
                    if ( resp != -1 ) {
                        return resp;
                    }
                }
                else if ( cont == C_DEL_APPT_BIN )
                {
                    fprintf(stderr, "Calling delAppt()\n" );
                    fflush(stderr);
                    int resp = delAppt();
                    if ( resp != -1 ) {
                        return resp;
                    }
                }
                else if ( cont == C_UPDATE_APPT_BIN )
                {
                    fprintf(stderr, "Calling updateAppt()\n" );
                    fflush(stderr);
                    int resp = updateAppt();
                    if ( resp != -1 ) {
                        return resp;
                    }
                }
            }
            return APP_RESPONSE::OK;
        //}
    }
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

    std::string control;

    // only check if the username exists if the name is valid
    control = std::to_string( OK );
    if ( ERRCHECKER::USERNAME( unamebuf ) && ERRCHECKER::PASSWORD( passbuf ) )
    {
        int ex = db.addUser( unamebuf, passbuf );
        int e = errno;
        if ( ex == -1 ) {
            if ( e == EEXIST ) {
                control = std::to_string( UNAME_USED );
            }
            else {
                control = std::to_string( ERR );
            }
        }
    }
    else
    {
        // Bad data
        control = std::to_string( MAL_DATA );
    }

    std::cout << control << std::endl;
    if ( ( send( new_fd, control.c_str(), MAXCONTROLSIZE, 0 ) ) == -1 )
    {
        int e = errno;
        std::cout << "(-1) errno -> " << e;
        return APP_RESPONSE::SEND;
    }

    return -1;
}

int app::login() {
    // response values
    //0000 0001 (1) – OK
    //0000 0010 (2) – Server Error
    //0000 0011 (3) – Username/password don’t exist
    //0000 0100 (4) - Malformed data

    const int OK = 0b0001;
    const int ERR = 0b0010;
    const int UNAMEPASS_NEX = 0b0011;
    const int MAL_DATA = 0b0100;

    char unamebuf[MAX_USERNAME_LENGTH] = { 0, };
    int numbytes = recv( new_fd, unamebuf, MAX_USERNAME_LENGTH, 0 );
    fprintf(stderr, "(2) numbytes = %d\n", numbytes);
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;

    printf("Received USERNAME from %s: %s\n",inet_ntoa(their_addr.sin_addr), unamebuf );

    char passbuf[MAX_USERNAME_LENGTH] = { 0, };
    numbytes = recv( new_fd, passbuf, MAX_PASSWORD_LENGTH, 0 );

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

    std::string control;

    // only check if the username exists if the name is valid
    control = std::to_string( OK );
    if ( ERRCHECKER::USERNAME( unamebuf ) && ERRCHECKER::PASSWORD( passbuf ) )
    {
        std::cout << "LOGGING IN" << std::endl;
        int ex = db.login( unamebuf, passbuf );
        int e = errno;
        if ( ex == -1 ) {
            if ( e == EEXIST ) {
                control = std::to_string( UNAMEPASS_NEX );
            }
            else {
                control = std::to_string( ERR );
            }
        }
    }
    else
    {
        // Bad data
        control = std::to_string( MAL_DATA );
    }

    std::cout << control << std::endl;
    if ( ( send( new_fd, control.c_str(), MAXCONTROLSIZE, 0 ) ) == -1 )
    {
        int e = errno;
        std::cout << "(-1) errno -> " << e;
        return APP_RESPONSE::SEND;
    }

    return -1;
}

int app::getAppts() {
    user_appointments.clear();
    int size = db.getAppts( user_appointments );
    int numbytes;

    if ( size != -1 ) {
        // send the number of appointments
        if ( ( send( new_fd, &size, sizeof(int), 0 ) ) == -1 )
        {
            return APP_RESPONSE::SEND;
        }
        std::cout << std::endl << std::endl;
        std::cout << "Num elements: " << std::to_string(size) << std::endl;

        for ( int i = 0; i < size; ++i) {
            if ( ( send( new_fd, &user_appointments[i].ID, sizeof(int), 0 ) ) == -1 ) {
                return APP_RESPONSE::SEND;
            }

            // ----------------------------------------------------------------------------------------------------------------------
            // send the size
            //uint32_t s = htonl( user_appointments[i].begin.length() + 1);
            int s = user_appointments[i].begin.length() + 1;
            if ( ( numbytes = send( new_fd, &s, sizeof(int), 0 ) ) == -1 ) {
                assert(numbytes == MAXCONTROLSIZE_DATA);
                return APP_RESPONSE::SEND;
            }
            // send the string
            if ( ( numbytes = send( new_fd, user_appointments[i].begin.c_str(), s, 0 ) ) == -1 ) {
                assert(numbytes == s);
                return APP_RESPONSE::SEND;
            }
            //std::cout << i << "-" << user_appointments[i].begin << std::endl;

            // ----------------------------------------------------------------------------------------------------------------------
            // send the size
            //s = htonl(user_appointments[i].end.length() + 1);
            s = user_appointments[i].end.length() + 1;
            if ( ( numbytes = send( new_fd, &s, sizeof(int), 0 ) ) == -1 ) {
                assert(numbytes == sizeof(int));
                return APP_RESPONSE::SEND;
            }
            // send the string
            if ( ( numbytes = send( new_fd, user_appointments[i].end.c_str(), s, 0 ) ) == -1 ) {
                assert(numbytes == s);
                return APP_RESPONSE::SEND;
            }
            //std::cout << i << "-" << user_appointments[i].end << std::endl;

            // ----------------------------------------------------------------------------------------------------------------------
            // send the size
            //s = htonl(user_appointments[i].place.length() + 1);
            s = user_appointments[i].place.length() + 1;
            if ( ( numbytes = send( new_fd, &s, sizeof(int), 0 ) ) == -1 ) {
                assert(numbytes == sizeof(int));
                return APP_RESPONSE::SEND;
            }
            // send the string
            if ( ( numbytes = send( new_fd, user_appointments[i].place.c_str(), s, 0 ) ) == -1 ) {
                assert(numbytes == s);
                return APP_RESPONSE::SEND;
            }
            //std::cout << i << "-" << user_appointments[i].place << std::endl;

            // ----------------------------------------------------------------------------------------------------------------------
            // send the size
            //s = htonl(user_appointments[i].contents.length() + 1);
            s = user_appointments[i].contents.length() + 1;
            if ( ( numbytes = send( new_fd, &s, sizeof(int), 0 ) ) == -1 ) {
                assert(numbytes == sizeof(int));
                return APP_RESPONSE::SEND;
            }
            // send the string
            if ( ( numbytes = send( new_fd, user_appointments[i].contents.c_str(), s, 0 ) ) == -1 ) {
                assert(numbytes == s);
                return APP_RESPONSE::SEND;
            }
            //std::cout << i << "-" << user_appointments[i].contents << std::endl;
        }
    }
    return -1;
}

int app::addAppt() {
    // response values
    // 0000 0001 (1) – OK
    // 0000 0010 (2) - ERR
    // 0000 0011 (3) – Malformed Data

    char beginbuf[APPT_BEGIN_END_LENGTH + 1] = {0, };
    char endbuf[APPT_BEGIN_END_LENGTH + 1] = {0, };
    char placebuf[APPT_PLACE_LENGTH + 1] = {0, };
    char contentsbuf[APPT_CONTENTS_LENGTH + 1] = {0, };

    //------------------------------------------------------------------------
    // get the appt begin
    char data_length_buf[32] = {0, };
    int numbytes = recv( new_fd, data_length_buf, MAXCONTROLSIZE_DATA, 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::OK; }
    numbytes = recv( new_fd, beginbuf, atoi(data_length_buf), 0 );
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;

    //------------------------------------------------------------------------
    // get the appt end
    bzero(data_length_buf, 0);
    numbytes = recv( new_fd, data_length_buf, MAXCONTROLSIZE_DATA, 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::OK; }
    numbytes = recv( new_fd, endbuf, atoi(data_length_buf), 0 );
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;

    //------------------------------------------------------------------------
    // get the appt place
    bzero(data_length_buf, 0);
    numbytes = recv( new_fd, data_length_buf, MAXCONTROLSIZE_DATA, 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::OK; }
    numbytes = recv( new_fd, placebuf, atoi(data_length_buf), 0 );
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;

    //------------------------------------------------------------------------
    // get the appt contents
    bzero(data_length_buf, 0);
    numbytes = recv( new_fd, data_length_buf, MAXCONTROLSIZE_DATA, 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::OK; }
    numbytes = recv( new_fd, contentsbuf, atoi(data_length_buf), 0 );
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;

    if ( db.addAppt( beginbuf, endbuf, placebuf, contentsbuf ) == 0 ) {
        return -1;
    }
    else {
        //send a response back
    }
}

int app::delAppt() {
    int id;

    int numbytes = recv( new_fd, &id, sizeof(int), 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::OK; }

    if ( db.delAppt( id ) != -1 ){
        return -1;
    }
    return -1;
}

int app::updateAppt() {
    int id;
    int numbytes = recv( new_fd, &id, sizeof(int), 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::OK; }

    char beginbuf[APPT_BEGIN_END_LENGTH + 1] = {0, };
    char endbuf[APPT_BEGIN_END_LENGTH + 1] = {0, };
    char placebuf[APPT_PLACE_LENGTH + 1] = {0, };
    char contentsbuf[APPT_CONTENTS_LENGTH + 1] = {0, };

    //------------------------------------------------------------------------
    // get the appt begin
    char data_length_buf[32] = {0, };
    numbytes = recv( new_fd, data_length_buf, MAXCONTROLSIZE_DATA, 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::OK; }
    numbytes = recv( new_fd, beginbuf, atoi(data_length_buf), 0 );
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;

    //------------------------------------------------------------------------
    // get the appt end
    bzero(data_length_buf, 0);
    numbytes = recv( new_fd, data_length_buf, MAXCONTROLSIZE_DATA, 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::OK; }
    numbytes = recv( new_fd, endbuf, atoi(data_length_buf), 0 );
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;

    //------------------------------------------------------------------------
    // get the appt place
    bzero(data_length_buf, 0);
    numbytes = recv( new_fd, data_length_buf, MAXCONTROLSIZE_DATA, 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::OK; }
    numbytes = recv( new_fd, placebuf, atoi(data_length_buf), 0 );
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;

    //------------------------------------------------------------------------
    // get the appt contents
    bzero(data_length_buf, 0);
    numbytes = recv( new_fd, data_length_buf, MAXCONTROLSIZE_DATA, 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::OK; }
    numbytes = recv( new_fd, contentsbuf, atoi(data_length_buf), 0 );
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;

    if ( db.updateAppt( id, beginbuf, endbuf, placebuf, contentsbuf ) != -1 ){
        return -1;
    }
    return -1;
}

int app::getUserData() {

}