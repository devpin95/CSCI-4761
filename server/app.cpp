#include <assert.h>
#include "app.h"

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

        if (!fork()) { // this is the child process
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

                //..... Do stuff
                if ( cont == C_DISCONN_BIN )
                {
                    //fprintf(stderr, "User disconnecting\n" );
                    return APP_RESPONSE::OK;
                }
                else if ( cont == C_ADD_USER_BIN ) {
                    //fprintf(stderr, "Calling addUser()\n" );
                    //fflush(stderr);
                    int resp = addUser();
                    if ( resp != -1 ) {
                        return resp;
                    }
                }
                else if ( cont == C_LOGIN_BIN )
                {
                    //fprintf(stderr, "Calling login()\n" );
                    int resp = login();
                    if ( resp != -1 ) {
                        return resp;
                    }
                }
                else if ( cont == C_GET_USER_APPTS_BIN )
                {
                    //fprintf(stderr, "Calling getAppts()\n" );
                    //fflush(stderr);
                    int resp = getAppts();
                    if ( resp != -1 ) {
                        return resp;
                    }
                }
                else if ( cont == C_ADD_APPT_BIN )
                {
                    //fprintf(stderr, "Calling addAppt()\n" );
                    //fflush(stderr);
                    int resp = addAppt();
                    if ( resp != -1 ) {
                        return resp;
                    }
                }
                else if ( cont == C_DEL_APPT_BIN )
                {
                    //fprintf(stderr, "Calling delAppt()\n" );
                    //fflush(stderr);
                    int resp = delAppt();
                    if ( resp != -1 ) {
                        return resp;
                    }
                }
                else if ( cont == C_UPDATE_APPT_BIN )
                {
                    //fprintf(stderr, "Calling updateAppt()\n" );
                    //fflush(stderr);
                    int resp = updateAppt();
                    if ( resp != -1 ) {
                        return resp;
                    }
                }
                else if ( cont == C_GET_USER_DATA_BIN )
                {
                    //fprintf(stderr, "Calling getUserData()\n" );
                    //fflush(stderr);
                    int resp = getUserData();
                    if ( resp != -1 ) {
                        return resp;
                    }
                }
                else if ( cont == C_UPDATE_USER_DATA_BIN )
                {
                    //fprintf(stderr, "Calling updateUserData()\n" );
                    //fflush(stderr);
                    int resp = updateUserData();
                    if ( resp != -1 ) {
                        return resp;
                    }
                }
            }
            return APP_RESPONSE::OK;
        }
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

    char unamebuf[UserData::MAX_USERNAME_LENGTH] = { 0, };
    // get the data size
    int data_length;
    int numbytes = recv( new_fd, &data_length, sizeof(int), 0 );
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;
    assert(numbytes==sizeof(int));
    // get the username
    numbytes = recv( new_fd, unamebuf, data_length, 0 );
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;
    assert(numbytes==data_length);

    char passbuf[UserData::MAX_PASSWORD_LENGTH] = { '\0', };
    // get the data size
    numbytes = recv( new_fd, &data_length, sizeof(int), 0 );
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;
    assert(numbytes==sizeof(int));

    numbytes = recv( new_fd, passbuf, data_length, 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::OK; }
    assert(numbytes==data_length);

    std::string control;

    // only check if the username exists if the name is valid
    control = std::to_string( OK );
    if ( UserData::USERNAME( unamebuf ) && UserData::PASSWORD( passbuf ) )
    {
        int ex = db.addUser( unamebuf, passbuf );
        int e = errno;
        if ( ex == -1 ) {
            if ( e == EEXIST ) {
                // username taken
                if ((numbytes=send(new_fd, &UNAME_USED, sizeof(int), 0)) == -1) {
                    return APP_RESPONSE::SEND;
                }
                assert( numbytes == sizeof(int) );
            }
            else {
                // err
                if ((numbytes=send(new_fd, &ERR, sizeof(int), 0)) == -1) {
                    return APP_RESPONSE::SEND;
                }
                assert( numbytes == sizeof(int) );
            }
        } else if ( ex == 0) {
            // ok
            if ((numbytes=send(new_fd, &OK, sizeof(int), 0)) == -1) {
                return APP_RESPONSE::SEND;
            }
            assert( numbytes == sizeof(int) );
        }
    }
    else
    {
        // Bad data
        if ((numbytes=send(new_fd, &MAL_DATA, sizeof(int), 0)) == -1) {
            return APP_RESPONSE::SEND;
        }
        assert( numbytes == sizeof(int) );
    }

    return -1;
}

int app::login() {
    // response values
    //0000 0001 (1) – OK
    //0000 0010 (2) – Server Error
    //0000 0011 (3) – Username/password don’t exist
    //0000 0100 (4) - Malformed data

    char unamebuf[UserData::MAX_USERNAME_LENGTH] = { 0, };
    char passbuf[UserData::MAX_PASSWORD_LENGTH] = { 0, };

    // get the data size
    int data_length;
    int numbytes = recv( new_fd, &data_length, sizeof(int), 0 );
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;
    assert(numbytes==sizeof(int));

    // get the data
    numbytes = recv( new_fd, unamebuf, data_length, 0 );
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;
    assert(numbytes==data_length);

    // get the data size
    numbytes = recv( new_fd, &data_length, sizeof(int), 0 );
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;
    assert(numbytes==sizeof(int));

    // get the data
    numbytes = recv( new_fd, passbuf, data_length, 0 );
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;
    assert(numbytes==data_length);
//
//    numbytes = recv( new_fd, passbuf, MAX_PASSWORD_LENGTH, 0 );
//
//    if ( numbytes == -1 ) {
//        int e = errno;
//        std::cout << "errno -> " << e;
//        return APP_RESPONSE::RECV;
//    }
//    else if ( numbytes == 0 ) {
//        int e = errno;
//        std::cout << "errno -> " << e;
//        return APP_RESPONSE::OK;
//    }


    std::string control;

    // response values
    //0000 0001 (1) – OK
    //0000 0010 (2) – Server Error
    //0000 0011 (3) – Username/password don’t exist
    //0000 0100 (4) - Malformed data

    const int OK = 0b0001;
    const int ERR = 0b0010;
    const int UNAMEPASS_NEX = 0b0011;
    const int MAL_DATA = 0b0100;

    // only check if the username exists if the name is valid
    if ( UserData::USERNAME( unamebuf ) && UserData::PASSWORD( passbuf ) )
    {
        int ex = db.login( unamebuf, passbuf );
        if ( ex == 0 )
        {
            // ok
            if ((numbytes=send(new_fd, &OK, sizeof(int), 0)) == -1) {
                return APP_RESPONSE::SEND;
            }
            assert( numbytes == sizeof(int) );

        }
        else if ( ex == 1 )
        {
                //username/password mismatch
                if ((numbytes=send(new_fd, &UNAMEPASS_NEX, sizeof(int), 0)) == -1) {
                    return APP_RESPONSE::SEND;
                }
                assert( numbytes == sizeof(int) );
        }
        else if ( ex == -1 )
        {
            //server error
            if ((numbytes=send(new_fd, &ERR, sizeof(int), 0)) == -1) {
                return APP_RESPONSE::SEND;
            }
            assert( numbytes == sizeof(int) );
        }
    }
    else
    {
        // Bad data
        if ((numbytes =send(new_fd, &MAL_DATA, sizeof(int), 0)) == -1) {
            return APP_RESPONSE::SEND;
        }
        assert( numbytes == sizeof(int) );
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
    int data_length;
    int numbytes = recv( new_fd, &data_length, sizeof(int), 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::OK; }
    numbytes = recv( new_fd, beginbuf, data_length, 0 );
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;

    //------------------------------------------------------------------------
    // get the appt end
    numbytes = recv( new_fd, &data_length, sizeof(int), 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::OK; }
    numbytes = recv( new_fd, endbuf, data_length, 0 );
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;

    //------------------------------------------------------------------------
    // get the appt place
    numbytes = recv( new_fd, &data_length, sizeof(int), 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::OK; }
    numbytes = recv( new_fd, placebuf, data_length, 0 );
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;

    //------------------------------------------------------------------------
    // get the appt contents
    numbytes = recv( new_fd, &data_length, sizeof(int), 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::OK; }
    numbytes = recv( new_fd, contentsbuf, data_length, 0 );
    if ( numbytes == -1 ) return APP_RESPONSE::RECV;
    else if ( numbytes == 0 ) return APP_RESPONSE::OK;

    // response values
    // 0000 0001 (1) – OK
    // 0000 0010 (2) - ERR
    // 0000 0011 (3) – Malformed Data
    const int OK = 0b0001;
    const int ERR = 0b0010;
    const int MAL = 0b0011;

    if ( Appt::TIME( beginbuf ) && Appt::TIME( endbuf ) && Appt::PLACE( placebuf ) && Appt::CONTENTS( contentsbuf ) ) {
        if (db.addAppt(beginbuf, endbuf, placebuf, contentsbuf) == 0)
        {
            // ok
            if ((send(new_fd, &OK, sizeof(int), 0)) == -1) {
                return APP_RESPONSE::SEND;
            }
        }
        else
        {
            // server error
            if ((send(new_fd, &ERR, sizeof(int), 0)) == -1) {
                return APP_RESPONSE::SEND;
            }
        }
    }
    else
    {
        //mal data
        if ((send(new_fd, &MAL, sizeof(int), 0)) == -1) {
            return APP_RESPONSE::SEND;
        }
    }

    return -1;
}

int app::delAppt() {
    // response values
    // 0000 0001 (1) – OK
    // 0000 0010 (2) - ERR
    // 0000 0011 (3) – NOEXIST
    // 0000 0100 (4) - MALFORMED DATA

    int id;

    int numbytes = recv( new_fd, &id, sizeof(int), 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::OK; }
    assert( numbytes == sizeof(int) );

    // response values
    // 0000 0001 (1) – OK
    // 0000 0010 (2) - ERR
    // 0000 0011 (3) – NOEXIST
    // 0000 0100 (4) - MALFORMED DATA
    const int OK = 0b0001;
    const int ERR = 0b0010;
    const int NOEXIST = 0b0011;
    const int MAL = 0b0100;

    if ( Appt::I( id ) ) {
        int re = db.delAppt( id );

        if ( re == 1 )
        {
            // ok
            if ((numbytes=send(new_fd, &OK, sizeof(int), 0)) == -1) {
                return APP_RESPONSE::SEND;
            }
            assert( numbytes == sizeof(int) );
        }
        else if ( re == 0 )
        {
            // no appt found with id
            if (numbytes = (send(new_fd, &NOEXIST, sizeof(int), 0)) == -1) {
                return APP_RESPONSE::SEND;
            }
            assert( numbytes == sizeof(int) );
        }
        else
        {
            // server error
            if ((numbytes=send(new_fd, &ERR, sizeof(int), 0)) == -1) {
                return APP_RESPONSE::SEND;
            }
            assert( numbytes == sizeof(int) );
        }
    }
    else
    {
        //mal data
        if ((numbytes =send(new_fd, &MAL, sizeof(int), 0)) == -1) {
            return APP_RESPONSE::SEND;
        }
        assert( numbytes == sizeof(int) );
    }

    return -1;
}

int app::updateAppt() {
    // response values
    // 0000 0001 (1) – OK
    // 0000 0010 (2) - ERR
    // 0000 0011 (3) – Malformed Data

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


    // response values
    // 0000 0001 (1) – OK
    // 0000 0010 (2) - ERR
    // 0000 0011 (3) - NOTFOUND
    // 0000 0100 (4) – Malformed Data
    const int OK = 0b0001;
    const int ERR = 0b0010;
    const int NOTFOUND = 0b0011;
    const int MAL = 0b0100;

    if ( Appt::TIME( beginbuf ) && Appt::TIME( endbuf ) && Appt::PLACE( placebuf ) && Appt::CONTENTS( contentsbuf ) ) {
        int re = db.updateAppt( id, beginbuf, endbuf, placebuf, contentsbuf );
        if ( re == 0 ){
            // ok
            if ((send(new_fd, &OK, sizeof(int), 0)) == -1) {
                return APP_RESPONSE::SEND;
            }
        }
        else if ( re == -1 )
        {
            // server error
            if ((send(new_fd, &ERR, sizeof(int), 0)) == -1) {
                return APP_RESPONSE::SEND;
            }
        }
        else if ( re == 1 )
        {
            // server error
            if ((send(new_fd, &NOTFOUND, sizeof(int), 0)) == -1) {
                return APP_RESPONSE::SEND;
            }
        }
    }
    else
    {
        //mal data
        if ((send(new_fd, &MAL, sizeof(int), 0)) == -1) {
            return APP_RESPONSE::SEND;
        }
    }

    return -1;
}

int app::getUserData() {
    UserData userdata;
    if ( db.getUserData( userdata ) == -1 ) {
        // return error
    }

    int numbytes;

    // send the size
    for ( char& c : userdata.password ) { c = '*'; }
    int s = userdata.password.length() + 1;
    if ( ( numbytes = send( new_fd, &s, sizeof(int), 0 ) ) == -1 ) {
        assert(numbytes == sizeof(int));
        return APP_RESPONSE::SEND;
    }
    // send the password
    if ( ( numbytes = send( new_fd, userdata.password.c_str(), s, 0 ) ) == -1 ) {
        assert(numbytes == s);
        return APP_RESPONSE::SEND;
    }

    // send the size
    s = userdata.name.length() + 1;
    if ( ( numbytes = send( new_fd, &s, sizeof(int), 0 ) ) == -1 ) {
        assert(numbytes == sizeof(int));
        return APP_RESPONSE::SEND;
    }
    // send the name
    if ( ( numbytes = send( new_fd, userdata.name.c_str(), s, 0 ) ) == -1 ) {
        assert(numbytes == s);
        return APP_RESPONSE::SEND;
    }

    // send the size
    s = userdata.email.length() + 1;
    if ( ( numbytes = send( new_fd, &s, sizeof(int), 0 ) ) == -1 ) {
        assert(numbytes == sizeof(int));
        return APP_RESPONSE::SEND;
    }
    // send the email
    if ( ( numbytes = send( new_fd, userdata.email.c_str(), s, 0 ) ) == -1 ) {
        assert(numbytes == s);
        return APP_RESPONSE::SEND;
    }

    // send the size
    s = userdata.phone.length() + 1;
    if ( ( numbytes = send( new_fd, &s, sizeof(int), 0 ) ) == -1 ) {
        assert(numbytes == sizeof(int));
        return APP_RESPONSE::SEND;
    }
    // send the phone
    if ( ( numbytes = send( new_fd, userdata.phone.c_str(), s, 0 ) ) == -1 ) {
        assert(numbytes == s);
        return APP_RESPONSE::SEND;
    }

    return -1;
}

int app::updateUserData() {

    db.getUserData(userdata);

    // SEC control
    const int C_UP_PASS = 0b0001;
    const int C_UP_NAME = 0b0010;
    const int C_UP_EMAIL = 0b0011;
    const int C_UP_PHONE = 0b0100;

    int sec_control;
    int numbytes = recv( new_fd, &sec_control, sizeof(int), 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::OK; }

    int data_length;
    char passbuf[UserData::MAX_PASSWORD_LENGTH+1] = {0, };
    char namebuf[UserData::MAX_NAME_LENGTH+1] = {0, };
    char emailbuf[UserData::MAX_EMAIL_LENGTH+1] = {0, };
    char phonebuf[UserData::MAX_PHONE_LENGTH+1] = {0, };

    bool valid = false;
    switch ( sec_control ) {
        case C_UP_PASS:
            //------------------------------------------------------------------------
            // get the appt begin
            numbytes = recv( new_fd, &data_length, sizeof(int), 0 );
            if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
            else if ( numbytes == 0 ) {
                return APP_RESPONSE::OK;
            }
            assert(numbytes==sizeof(int));

            numbytes = recv( new_fd, passbuf, data_length, 0 );
            if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
            else if ( numbytes == 0 ) {
                return APP_RESPONSE::OK;
            }
            assert(numbytes==data_length);
            //------------------------------------------------------------------------

            if ( UserData::PASSWORD( passbuf ) ) {
                valid = true;
                userdata.password = passbuf;
            }

            break;
        case C_UP_NAME:
            //------------------------------------------------------------------------
            // get the appt begin
            numbytes = recv( new_fd, &data_length, sizeof(int), 0 );
            if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
            else if ( numbytes == 0 ) {
                return APP_RESPONSE::OK;
            }
            assert(numbytes==sizeof(int));

            numbytes = recv( new_fd, namebuf, data_length, 0 );
            if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
            else if ( numbytes == 0 ) {
                return APP_RESPONSE::OK;
            }
            assert(numbytes==data_length);
            //------------------------------------------------------------------------

            if ( UserData::NAME( namebuf ) ) {
                valid = true;
                userdata.name = namebuf;
            }
            break;
        case C_UP_EMAIL:
            //------------------------------------------------------------------------
            // get the appt begin
            numbytes = recv( new_fd, &data_length, sizeof(int), 0 );
            if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
            else if ( numbytes == 0 ) {
                return APP_RESPONSE::OK;
            }
            assert(numbytes==sizeof(int));

            numbytes = recv( new_fd, emailbuf, data_length, 0 );
            if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
            else if ( numbytes == 0 ) {
                return APP_RESPONSE::OK;
            }
            assert(numbytes==data_length);
            //------------------------------------------------------------------------

            if ( UserData::EMAIL( emailbuf ) ) {
                valid = true;
                userdata.email = emailbuf;
            }
            break;
        case C_UP_PHONE:
            //------------------------------------------------------------------------
            // get the appt begin
            numbytes = recv( new_fd, &data_length, sizeof(int), 0 );
            if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
            else if ( numbytes == 0 ) {
                return APP_RESPONSE::OK;
            }
            assert(numbytes==sizeof(int));

            numbytes = recv( new_fd, phonebuf, data_length, 0 );
            if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
            else if ( numbytes == 0 ) {
                return APP_RESPONSE::OK;
            }
            assert(numbytes==data_length);
            //------------------------------------------------------------------------

            if ( UserData::PHONE( phonebuf ) ) {
                valid = true;
                userdata.phone = phonebuf;
            }
            break;
        default:
            break;
    }

    // response values
    // 0000 0001 (1) – OK
    // 0000 0010 (2) - ERR
    // 0000 0011 (3) – NOEXIST
    // 0000 0100 (4) - MALFORMED DATA
    const int OK = 0b0001;
    const int ERR = 0b0010;
    const int MAL = 0b0011;

    if ( valid ) {
        int re = db.updateUserData(userdata);

        if ( re == 0 ) {
            // ok
            if ((numbytes=send(new_fd, &OK, sizeof(int), 0)) == -1) {
                return APP_RESPONSE::SEND;
            }
            assert( numbytes == sizeof(int) );
        }
        else if ( re == -1 ) {
            //server error
            if ((numbytes=send(new_fd, &ERR, sizeof(int), 0)) == -1) {
                return APP_RESPONSE::SEND;
            }
            assert( numbytes == sizeof(int) );
        }
    }
    else {
        // mal data
        if ((numbytes=send(new_fd, &MAL, sizeof(int), 0)) == -1) {
            return APP_RESPONSE::SEND;
        }
        assert( numbytes == sizeof(int) );
    }

    return -1;
}