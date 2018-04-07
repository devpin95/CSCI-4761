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
            int resp = login();
            if (resp == -1) {
                choice = 0;

                while ( choice != 5 ) {
                    std::cout << "1. View Appointments" << std::endl
                              << "2. Add Appointment" << std::endl
                              << "3. Remove Appointment" << std::endl
                              << "4. Update Appointment" << std::endl
                              << "5. Log out" << std::endl
                              << ">";

                    std::cin >> choice;

                    switch (choice) {
                        case 1:
                            resp = viewAppts();
                            // check if the request was successfuly
                            if ( resp != -1 ) {
                                return resp;
                            }
                            // check if the user has any appointments
                            if ( !user_appointments.empty() ) {
                                // print out the appointments
                                for (Appt a : user_appointments) {
                                    //std::cout << user_appointments[i];
                                    a.operator<<(std::cout);
                                }
                            }
                            else {
                                // the user doesn't have any appointments
                                std::cout << "No Appointments Scheduled" << std::endl;
                            }
                            break;
                        case 2:
                            resp = addAppt();
                            // check if the request was successful
                            if ( resp != -1 ) {
                                return resp;
                            }
                            break;
                        case 3:
                            resp = delAppt();
                            // check if the request was successful
                            if ( resp != -1 ) {
                                return resp;
                            }
                            break;
                        case 4:
                            break;
                        default: break;
                    }
                }
            }
            else {
                return resp;
            }
        }
        else if ( choice == 2 ) {
            //control = C_ADD_USER.c_str();
            int resp = addUser();
            if (resp == -1) {
                return resp;
            }
        }
        else if ( choice == 3 ) {
            return APP_RESPONSE::DISCONN;
        }
    }
}

int app::login() {
    // response values
    // 0000 0001 (1) – DISCONN
    // 0000 0010 (2) – Server Error
    // 0000 0011 (3) – Username already used
    // 0000 0100 (4) – Malformed Data
    const int OK = 0b0001;
    const int ERR = 0b0010;
    const int UNAME_USED = 0b0011;
    const int MAL_DATA = 0b0100;

    const char* control;
    char servresp[1] = {0};
    char unamebuf[MAX_USERNAME_LENGTH] = {'\0', };
    char passbuf[MAX_PASSWORD_LENGTH] = {'\0', };
    control = C_LOGIN.c_str();

    //send control byte
    if ((send(sockfd, control, /* sizeof(char) * strlen(control) */ MAXCONTROLSIZE, 0)) == -1) {
        int e = errno;
        std::cout << "errno -> " << e;
        return APP_RESPONSE::SEND;
    }


    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    while(true) {
        bzero(unamebuf, sizeof(unamebuf));

        std::cout << "User name: ";
        gets(unamebuf);
        unamebuf[sizeof(unamebuf)] = '\0';

        if ( ERRCHECKER::USERNAME(unamebuf) ) {
            break;
        } else {
            std::cout << unamebuf << " is not a valid username" << std::endl;
        }
    }

    //std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    while(true) {
        bzero(passbuf, sizeof(passbuf));

        std::cout << "Password: ";
        gets(passbuf);

        if ( ERRCHECKER::PASSWORD(passbuf) ) {
            break;
        } else {
            std::cout << "Invalid password" << std::endl;
        }
    }

    encrypt( passbuf, (uint)strlen(passbuf) );

    if ((send(sockfd, unamebuf, MAX_USERNAME_LENGTH, 0)) == -1) {
        int e = errno;
        std::cout << "errno -> " << e;
        return APP_RESPONSE::SEND;
    }

    if ((send(sockfd, passbuf, MAX_PASSWORD_LENGTH, 0)) == -1) {
        int e = errno;
        std::cout << "errno -> " << e;
        return APP_RESPONSE::SEND;
    }

    int numbytes = recv( sockfd, servresp, MAXCONTROLSIZE, 0 );
    int onrre = errno;

    if ( numbytes == -1 ) {
        perror("recv");
        int e = errno;
        std::cout << "(1) errno -> " << e << std::endl;
        return APP_RESPONSE::RECV;
    }
    else if ( numbytes == 0 ) {
        int e = errno;
        std::cout << "(2) errno -> " << e;
        return APP_RESPONSE::DISCONN;
    }

    switch ( atoi( servresp ) ) {
        case OK:
            std::cout << std::endl << "Logged in successfully" << std::endl << std::endl;
            return -1;
        case ERR:
            std::cout << std::endl << "Server Error. Try Again." << std::endl << std::endl;
            break;
        case UNAME_USED:
            std::cout << "That username is already taken. Try a different name." << std::endl;
            break;
        case MAL_DATA:
            std::cout << "Username or password are not valid. Try again." << std::endl;
            break;
        default:
            std::cout << "Something went wrong" << std::endl;
            break;
    }
    return -2;
}

int app::addUser() {
    // response values
    // 0000 0001 (1) – DISCONN
    // 0000 0010 (2) – Server Error
    // 0000 0011 (3) – Username already used
    // 0000 0100 (4) – Malformed Data
    const int OK = 0b0001;
    const int ERR = 0b0010;
    const int UNAME_USED = 0b0011;
    const int MAL_DATA = 0b0100;

    const char* control;
    char servresp[1] = {0};
    char unamebuf[MAX_USERNAME_LENGTH] = {'\0', };
    char passbuf[MAX_PASSWORD_LENGTH] = {'\0', };
    control = C_ADD_USER.c_str();

    //send control byte
    if ((send(sockfd, control, /* sizeof(char) * strlen(control) */ MAXCONTROLSIZE, 0)) == -1) {
        int e = errno;
        std::cout << "errno -> " << e;
        return APP_RESPONSE::SEND;
    }


    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

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

    //std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

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

    encrypt( passbuf, (uint)strlen(passbuf) );

    if ((send(sockfd, unamebuf, MAX_USERNAME_LENGTH, 0)) == -1) {
        int e = errno;
        std::cout << "errno -> " << e;
        return APP_RESPONSE::SEND;
    }

    if ((send(sockfd, passbuf, MAX_USERNAME_LENGTH, 0)) == -1) {
        int e = errno;
        std::cout << "errno -> " << e;
        return APP_RESPONSE::SEND;
    }

    int numbytes = recv( sockfd, servresp, MAXCONTROLSIZE, 0 );
    int onrre = errno;

    if ( numbytes == -1 ) {
        perror("recv");
        int e = errno;
        std::cout << "(1) errno -> " << e << std::endl;
        return APP_RESPONSE::RECV;
    }
    else if ( numbytes == 0 ) {
        int e = errno;
        std::cout << "(2) errno -> " << e;
        return APP_RESPONSE::DISCONN;
    }

    switch ( atoi( servresp ) ) {
        case OK:
            std::cout << "Account Added. Please log in with your credentials." << std::endl;
            break;
        case ERR:
            std::cout << "Server Error. Try Again." << std::endl;
            break;
        case UNAME_USED:
            std::cout << "That username is already taken. Try a different name." << std::endl;
            break;
        case MAL_DATA:
            std::cout << "Username or password are not valid. Try again." << std::endl;
            break;
        default:
            std::cout << "Something went wrong" << std::endl;
            break;
    }
}

int app::viewAppts() {
    const char* control;
    char num_appts[MAX_APPTS_SIZE_LENGTH] = {0, };
    int appt_id;
    char appt_begin[APPT_BEGIN_END_LENGTH + 1] = {0, };
    char appt_end[APPT_BEGIN_END_LENGTH + 1] = {0, };
    char appt_place[APPT_PLACE_LENGTH + 1] = {0, };
    char appt_contents[APPT_CONTENTS_LENGTH + 1] = {0, };
    int size;
    control = C_GET_USER_APPTS.c_str();

    // send control byte
    if ((send(sockfd, control, MAXCONTROLSIZE, 0)) == -1) {
        return APP_RESPONSE::SEND;
    }

    // get the number of appointments that will be sent by the server
    int numbytes = recv( sockfd, num_appts, MAX_APPTS_SIZE_LENGTH, 0 );

    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::DISCONN; }

    //convert the buffer to an integer
    size = atoi(num_appts);

    // get the appointment details
    for ( int i = 0; i < size; ++i ) {

        //------------------------------------------------------------------------
        // get the appt id
        int numbytes = recv( sockfd, &appt_id, sizeof(int), 0 );

        if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
        else if ( numbytes == 0 ) { return APP_RESPONSE::DISCONN; }
        //------------------------------------------------------------------------




        //------------------------------------------------------------------------
        // get the appt begin
        char data_length_buf[32] = {0, };
        numbytes = recv( sockfd, data_length_buf, MAXCONTROLSIZE_DATA, 0 );

        if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
        else if ( numbytes == 0 ) { return APP_RESPONSE::DISCONN; }

        numbytes = recv( sockfd, appt_begin, atoi(data_length_buf), 0 );

        if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
        else if ( numbytes == 0 ) { return APP_RESPONSE::DISCONN; }
        //------------------------------------------------------------------------




        //------------------------------------------------------------------------
        // get the appt end
        bzero(data_length_buf, 0);
        numbytes = recv( sockfd, data_length_buf, MAXCONTROLSIZE_DATA, 0 );

        if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
        else if ( numbytes == 0 ) { return APP_RESPONSE::DISCONN; }

        numbytes = recv( sockfd, appt_end, atoi(data_length_buf), 0 );

        if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
        else if ( numbytes == 0 ) { return APP_RESPONSE::DISCONN; }
        //------------------------------------------------------------------------




        //------------------------------------------------------------------------
        // get the appt place
        bzero(data_length_buf, 0);
        numbytes = recv( sockfd, data_length_buf, MAXCONTROLSIZE_DATA, 0 );

        if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
        else if ( numbytes == 0 ) { return APP_RESPONSE::DISCONN; }

        numbytes = recv( sockfd, appt_place, atoi(data_length_buf), 0 );

        if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
        else if ( numbytes == 0 ) { return APP_RESPONSE::DISCONN; }
        //------------------------------------------------------------------------




        //------------------------------------------------------------------------
        // get the appt contents
        bzero(data_length_buf, 0);
        numbytes = recv( sockfd, data_length_buf, MAXCONTROLSIZE_DATA, 0 );

        if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
        else if ( numbytes == 0 ) { return APP_RESPONSE::DISCONN; }

        numbytes = recv( sockfd, appt_contents, atoi(data_length_buf), 0 );

        if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
        else if ( numbytes == 0 ) { return APP_RESPONSE::DISCONN; }
        //------------------------------------------------------------------------

        Appt appt;
        appt.ID = appt_id;
        appt.begin = appt_begin;
        appt.end = appt_end;
        appt.place = appt_place;
        appt.contents = appt_contents;

        user_appointments.push_back(appt);
    }

    return -1;
}

int app::addAppt() {
    const char* control;
    control = C_ADD_APPT.c_str();

    // send control byte
    if ((send(sockfd, control, MAXCONTROLSIZE, 0)) == -1) {
        int e = errno;
        std::cout << "errno -> " << e;
        return APP_RESPONSE::SEND;
    }

    int hh, mm, MM, DD, YYYY;
    std::cout << std::endl << "Begin Time (hh:mm MM/DD/YYYY 24-hour model)" << std::endl;
    std::cout << "hh: ";
    std::cin >> hh;
    std::cout << "mm: ";
    std::cin >> mm;
    std::cout << "MM: ";
    std::cin >> MM;
    std::cout << "DD: ";
    std::cin >> DD;
    std::cout << "YYYY: ";
    std::cin >> YYYY;

    std::string _hh, _mm, _MM, _DD, _YYYY;
    _hh = std::to_string(hh);
    _mm = std::to_string(mm);
    _MM = std::to_string(MM);
    _DD = std::to_string(DD);
    _YYYY = std::to_string(YYYY);

    std::string beginstr = _hh + ":" + _mm + " " + _MM + "/" + _DD + "/" + _YYYY;

    std::cout << std::endl << "End Time (hh:mm MM/DD/YYYY 24-hour model)" << std::endl;
    std::cout << "hh: ";
    std::cin >> hh;
    std::cout << "mm: ";
    std::cin >> mm;
    std::cout << "MM: ";
    std::cin >> MM;
    std::cout << "DD: ";
    std::cin >> DD;
    std::cout << "YYYY: ";
    std::cin >> YYYY;
    std::cin.ignore();

    _hh = std::to_string(hh);
    _mm = std::to_string(mm);
    _MM = std::to_string(MM);
    _DD = std::to_string(DD);
    _YYYY = std::to_string(YYYY);

    std::string endstr = _hh + ":" + _mm + " " + _MM + "/" + _DD + "/" + _YYYY;

    std::cout << std::endl << "Place ( " << APPT_PLACE_LENGTH << " characters or fewer ): ";
    //std::cin.ignore();
    std::string place;
    getline(std::cin, place);

    std::cout << std::endl << "Description ( " << APPT_CONTENTS_LENGTH << " characters or fewer ): ";
    std::string contents;
    getline(std::cin, contents);

    // ----------------------------------------------------------------------------------------------------------------------
    // send the size
    if ( ( send( sockfd, std::to_string(beginstr.length() + 1).c_str(), MAXCONTROLSIZE_DATA, 0 ) ) == -1 ) {
        return APP_RESPONSE::SEND;
    }
    // send the begin time
    if ( ( send( sockfd, beginstr.c_str(), beginstr.length() + 1, 0 ) ) == -1 ) {
        return APP_RESPONSE::SEND;
    }

    // ----------------------------------------------------------------------------------------------------------------------
    // send the size
    if ( ( send( sockfd, std::to_string(endstr.length() + 1).c_str(), MAXCONTROLSIZE_DATA, 0 ) ) == -1 ) {
        return APP_RESPONSE::SEND;
    }
    // send the end time
    if ( ( send( sockfd, endstr.c_str(), endstr.length() + 1, 0 ) ) == -1 ) {
        return APP_RESPONSE::SEND;
    }

    // ----------------------------------------------------------------------------------------------------------------------
    // send the size
    if ( ( send( sockfd, std::to_string(place.length() + 1).c_str(), MAXCONTROLSIZE_DATA, 0 ) ) == -1 ) {
        return APP_RESPONSE::SEND;
    }
    // send the place
    if ((send(sockfd, place.c_str(), place.length() + 1, 0)) == -1) {
        return APP_RESPONSE::SEND;
    }

    // ----------------------------------------------------------------------------------------------------------------------
    // send the size
    if ( ( send( sockfd, std::to_string(contents.length() + 1).c_str(), MAXCONTROLSIZE_DATA, 0 ) ) == -1 ) {
        return APP_RESPONSE::SEND;
    }
    // send the contents
    if ((send(sockfd, contents.c_str(), contents.length() + 1, 0)) == -1) {
        return APP_RESPONSE::SEND;
    }

    return -1;
}

int app::delAppt() {
    const char* control;
    control = C_DEL_APPT.c_str();

    // send control byte
    if ((send(sockfd, control, MAXCONTROLSIZE, 0)) == -1) {
        return APP_RESPONSE::SEND;
    }

    int id;
    std::cout << std::endl << "Appointment ID:";
    std::cin >> id;

    // send control byte
    if ((send(sockfd, &id, sizeof(int), 0)) == -1) {
        return APP_RESPONSE::SEND;
    }
}

void app::encrypt(char s[], uint len ) {
    for ( int i = 0; i < len; ++i ) {
        s[i] = s[i] + 13;
    }
}