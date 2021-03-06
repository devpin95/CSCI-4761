#include <assert.h>
#include "app.h"

int app::start(std::string &msg, int argc, char *argv[]) {
    struct hostent *he;
    struct sockaddr_in their_addr; // connector's address information

    if (argc != 2) {
        rlutil::saveDefaultColor();
        rlutil::setColor(rlutil::RED);
        std::cout << "usage: client hostname" << std::endl;
        rlutil::resetColor();
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

    rlutil::saveDefaultColor();
    rlutil::setColor(rlutil::GREEN);
    std::cout << "connection has been established with server." << std::endl;
    rlutil::resetColor();

    for(;;) {
        int choice;
        std::cout << std::endl << "1. Log In" << std::endl << "2. Create Account" << std::endl
                  << "3. Exit" << std::endl << "> ";

            std::cin >> choice;

        if (choice == 1) {
            int resp = login();
            if (resp == -1) {
                choice = 0;

                while ( choice != 6 ) {
                    std::cout << "1. View Appointments" << std::endl
                              << "2. Add Appointment" << std::endl
                              << "3. Remove Appointment" << std::endl
                              << "4. Update Appointment" << std::endl
                              << "5. Update Information" << std::endl
                              << "6. Log out" << std::endl
                              << ">";

                    std::cin >> choice;

                    switch (choice) {
                        case 1:
                            resp = viewAppts();
                            // check if the request was successfuly
                            if ( resp != -1 ) {
                                return resp;
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
                            resp = updateAppt();
                            // check if the request was successful
                            if ( resp != -1 ) {
                                return resp;
                            }
                            break;
                        case 5:
                            resp = updateUserInfo();
                            // check if the request was successful
                            if ( resp != -1 ) {
                                return resp;
                            }
                            break;
                        default: break;
                    }
                }
            }
            else if ( resp != -2 ){
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

    std::string unamebuf, passbuf;

    //send control byte
    if ((send(sockfd, &C_LOGIN_BIN, sizeof(int), 0)) == -1) {
        return APP_RESPONSE::SEND;
    }


    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    while(true) {

        std::cout << "User name: ";
        getline(std::cin, unamebuf);

        if ( UserData::USERNAME(unamebuf) ) {
            break;
        } else {
            rlutil::saveDefaultColor();
            rlutil::setColor(rlutil::RED);
            std::cout << unamebuf << " is not a valid username" << std::endl;
            rlutil::resetColor();
        }
    }

    //std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    while(true) {
        std::cout << "Password: ";
        getline(std::cin, passbuf);

        if ( UserData::PASSWORD(passbuf) ) {
            break;
        } else {
            rlutil::saveDefaultColor();
            rlutil::setColor(rlutil::RED);
            std::cout << "Invalid password" << std::endl;
            rlutil::resetColor();
        }
    }

    encrypt( passbuf, (uint)passbuf.length() );

    int numbytes;
    int data_size;
    // send the username
    data_size = unamebuf.length() + 1;
    if ((numbytes=send(sockfd, &data_size, sizeof(int), 0)) == -1) { return APP_RESPONSE::SEND; }
    assert(numbytes==sizeof(int));
    if ((send(sockfd, unamebuf.c_str(), data_size, 0)) == -1) { return APP_RESPONSE::SEND; }

    // send the password
    data_size = passbuf.length() + 1;
    if ((send(sockfd, &data_size, sizeof(int), 0)) == -1) { return APP_RESPONSE::SEND; }
    assert(numbytes==sizeof(int));
    if ((send(sockfd, passbuf.c_str(), data_size, 0)) == -1) { return APP_RESPONSE::SEND; }

    int re;
    // get the server response
    numbytes = recv( sockfd, &re, sizeof(int), 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::DISCONN; }
    assert(numbytes == sizeof(int));

    // response values
    // 0000 0001 (1) – DISCONN
    // 0000 0010 (2) – Server Error
    // 0000 0011 (3) – Username already used
    // 0000 0100 (4) – Malformed Data
    const int OK = 0b0001;
    const int ERR = 0b0010;
    const int UNAME_USED = 0b0011;
    const int MAL_DATA = 0b0100;

    switch ( re ) {
        case OK:
            rlutil::saveDefaultColor();
            rlutil::setColor(rlutil::GREEN);
            std::cout << std::endl << "Logged in successfully" << std::endl << std::endl;
            rlutil::resetColor();
            return -1;
        case ERR:
            rlutil::saveDefaultColor();
            rlutil::setColor(rlutil::RED);
            std::cout << std::endl << "Server Error. Try Again." << std::endl << std::endl;
            rlutil::resetColor();
            break;
        case UNAME_USED:
            rlutil::saveDefaultColor();
            rlutil::setColor(rlutil::RED);
            std::cout << std::endl << "Username/Password do not match." << std::endl;
            rlutil::resetColor();
            break;
        case MAL_DATA:
            rlutil::saveDefaultColor();
            rlutil::setColor(rlutil::RED);
            std::cout << "Username or password are not valid. Try again." << std::endl;
            rlutil::resetColor();
            break;
        default:
            rlutil::saveDefaultColor();
            rlutil::setColor(rlutil::RED);
            std::cout << "Something went wrong" << std::endl;
            rlutil::resetColor();
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

    std::string unamebuf, passbuf;

    //send control byte
    if ((send(sockfd, &C_ADD_USER_BIN, sizeof(int), 0)) == -1) {
        return APP_RESPONSE::SEND;
    }


    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    while(true) {

        std::cout << "User name (up to 20 characters): ";
        getline(std::cin, unamebuf);

        if ( UserData::USERNAME(unamebuf) ) {
            break;
        } else {
            std::cout << unamebuf << " is not a valid username" << std::endl;
        }
    }

    //std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    while(true) {

        std::cout << "Password (8-16 characters): ";
        getline(std::cin, passbuf);

        if ( UserData::PASSWORD(passbuf) ) {
            break;
        } else {
            std::cout << "Password must be 8-16 characters" << std::endl;
        }
    }

    encrypt( passbuf, (uint)passbuf.length() );

    // send the size
    int data_size = unamebuf.length() + 1;
    if ((send(sockfd, &data_size, sizeof(int), 0)) == -1) {
        return APP_RESPONSE::SEND;
    }
    // send the string
    if ((send(sockfd, unamebuf.c_str(), data_size, 0)) == -1) {
        return APP_RESPONSE::SEND;
    }

    // send the size
    data_size = passbuf.length() + 1;
    if ((send(sockfd, &data_size, sizeof(int), 0)) == -1) {
        return APP_RESPONSE::SEND;
    }
    // send the string
    if ((send(sockfd, passbuf.c_str(), data_size, 0)) == -1) {
        return APP_RESPONSE::SEND;
    }

    int servresp;
    int numbytes = recv( sockfd, &servresp, sizeof(int), 0 );

    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::DISCONN; }

    switch ( servresp ) {
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
    user_appointments.clear();
    int num_appts;
    int appt_id;
    char appt_begin[APPT_BEGIN_END_LENGTH + 1] = {0, };
    char appt_end[APPT_BEGIN_END_LENGTH + 1] = {0, };
    char appt_place[APPT_PLACE_LENGTH + 1] = {0, };
    char appt_contents[APPT_CONTENTS_LENGTH + 1] = {0, };
    int size;

    // send control byte
    if ((send(sockfd, &C_GET_USER_APPTS_BIN, sizeof(int), 0)) == -1) {
        return APP_RESPONSE::SEND;
    }

    // get the number of appointments that will be sent by the server
    int numbytes = recv( sockfd, &num_appts, sizeof(int), 0 );

    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { std::cout << "1" << std::endl; return APP_RESPONSE::DISCONN; }

    //convert the buffer to an integer
    size = num_appts;

    // get the appointment details
    for ( int i = 0; i < size; ++i ) {

        //------------------------------------------------------------------------
        // get the appt id
        int numbytes = recv( sockfd, &appt_id, sizeof(int), 0 );

        if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
        else if ( numbytes == 0 ) {
            std::cout << i << "-" << "2" << std::endl;
            return APP_RESPONSE::DISCONN;
        }
        //------------------------------------------------------------------------




        //------------------------------------------------------------------------
        // get the appt begin
        int data_length;
        numbytes = recv( sockfd, &data_length, sizeof(int), 0 );

        if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
        else if ( numbytes == 0 ) {
            std::cout << i << "-" << "3" << std::endl;
            return APP_RESPONSE::DISCONN;
        }
        assert(numbytes==sizeof(int));

        numbytes = recv( sockfd, appt_begin, data_length, 0 );

        if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
        else if ( numbytes == 0 ) {
            std::cout << i << "-" << "4" << std::endl;
            return APP_RESPONSE::DISCONN;
        }
        assert(numbytes==data_length);
        //------------------------------------------------------------------------




        //------------------------------------------------------------------------
        // get the appt end
        numbytes = recv( sockfd, &data_length, sizeof(int), 0 );
        //data_length = ntohl(data_length);

        if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
        else if ( numbytes == 0 ) {
            std::cout << i << "-" << "5" << std::endl;
            return APP_RESPONSE::DISCONN;
        }
        assert(numbytes==sizeof(int));

        numbytes = recv( sockfd, appt_end, data_length, 0 );

        if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
        else if ( numbytes == 0 ) {
            std::cout << i << "-" << "6" << std::endl;
            return APP_RESPONSE::DISCONN;
        }
        assert(numbytes==data_length);
        //------------------------------------------------------------------------




        //------------------------------------------------------------------------
        // get the appt place
        numbytes = recv( sockfd, &data_length, sizeof(uint32_t), 0 );
        //data_length = ntohl(data_length);

        if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
        else if ( numbytes == 0 ) {
            std::cout << i << "-" << "7" << std::endl;
            return APP_RESPONSE::DISCONN;
        }
        assert(numbytes==sizeof(int));

        numbytes = recv( sockfd, appt_place, data_length, 0 );

        if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
        else if ( numbytes == 0 ) {
            std::cout << i << "-" << "8" << std::endl;
            return APP_RESPONSE::DISCONN;
        }
        assert(numbytes==data_length);
        //------------------------------------------------------------------------




        //------------------------------------------------------------------------
        // get the appt contents
        numbytes = recv( sockfd, &data_length, sizeof(uint32_t), 0 );
        //data_length = ntohl(data_length);

        if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
        else if ( numbytes == 0 ) {
            std::cout << i << "-" << "9" << std::endl;
            return APP_RESPONSE::DISCONN;
        }
        assert(numbytes==sizeof(int));

        numbytes = recv( sockfd, appt_contents, data_length, 0 );

        if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
        else if ( numbytes == 0 ) {
            std::cout << i << "-" << "10" << std::endl;
            return APP_RESPONSE::DISCONN;
        }
        assert(numbytes==data_length);
        //------------------------------------------------------------------------

        Appt appt;
        appt.ID = appt_id;
        appt.begin = appt_begin;
        appt.end = appt_end;
        appt.place = appt_place;
        appt.contents = appt_contents;

        user_appointments.push_back(appt);
    }

    int choice;
    do {
        std::cout << std::endl << "1. View all appointments" << std::endl
                  << "2. Find appointment by time" << std::endl
                  << "3. Find appointments in time range" << std::endl
                  << "4. Cancel" << std::endl
                  << ">";

        std::cin >> choice;
    } while ( choice < 1 || choice > 4 );

    switch ( choice ) {
        case 1:
            std::sort( user_appointments.begin(), user_appointments.end() );

            // check if the user has any appointments
            if ( !user_appointments.empty() ) {
                // print out the appointments
                std::cout << std::endl << "Your schedule" << std::endl;

                rlutil::saveDefaultColor();
                rlutil::setColor(rlutil::RED);
                std::cout << "Red appointments indicate a schedule collision." << std::endl;
                rlutil::resetColor();

                for (Appt& a : user_appointments) {
                    auto color = rlutil::CYAN;
                    for ( Appt& b : user_appointments ) {
                        // dont compare the same appointments
                        if ( a == b ) {
                            continue;
                        }
                        // If the appt starts before the range, but ends after the beginning of the range
                        if ( ( Appt::timeLessThan(a.begin, b.begin) && Appt::timeLessThan(b.begin, a.end) ) ||
                             ( Appt::timeLessThan(b.begin, a.begin) && Appt::timeLessThan(a.begin, b.end) ) ) {
                            color = rlutil::RED;
                            break;
                        }
                    }
                    rlutil::saveDefaultColor();
                    rlutil::setColor(color);
                    a.operator<<(std::cout);
                    rlutil::resetColor();
                }
            }
            else {
                // the user doesn't have any appointments
                rlutil::saveDefaultColor();
                rlutil::setColor(rlutil::RED);
                std::cout << std::endl << "No Appointments Scheduled" << std::endl << std::endl;
                rlutil::resetColor();
            }
            break;
        case 2:
            viewApptsByTime();
            break;
        case 3:
            viewApptsByRange();
            break;
        default: break;
    }

    return -1;
}

int app::viewApptsByTime() {
    int hh, mm, MM, DD, YYYY;
    std::cout << std::endl << "Begin Time (hh:mm MM/DD/YYYY 24-hour model)" << std::endl;

    do {
        std::cout << "hh: ";
        std::cin >> hh;
    } while ( !ERRCHECKER::HOUR(hh) );

    do {
        std::cout << "mm: ";
        std::cin >> mm;
    } while ( !ERRCHECKER::MINUTE(mm) );

    do {
        std::cout << "MM: ";
        std::cin >> MM;
    } while ( !ERRCHECKER::MONTH(MM) );

    do {
        std::cout << "DD: ";
        std::cin >> DD;
    } while ( !ERRCHECKER::DAY(DD) );

    do {
        std::cout << "YYYY: ";
        std::cin >> YYYY;
    } while ( !ERRCHECKER::YEAR(YYYY) );

    std::string _hh, _mm, _MM, _DD, _YYYY;
    _hh = std::to_string(hh);
    if ( hh == 0 ) {
        _hh = "00";
    }
    else if ( hh < 10 ) {
        _hh = "0" + _hh;
    }

    _mm = std::to_string(mm);
    if ( mm == 0 ) {
        _mm = "00";
    }
    else if ( mm < 10 ) {
        _mm = "0" + _hh;
    }

    _MM = std::to_string(MM);
    if ( MM < 10 ) {
        _MM = "0" + _MM;
    }

    _DD = std::to_string(DD);
    if ( DD < 10 ) {
        _DD = "0" + _DD;
    }

    _YYYY = std::to_string(YYYY);

    std::string beginstr = _hh + ":" + _mm + " " + _MM + "/" + _DD + "/" + _YYYY;

    std::vector<Appt> temp_vector;
    for ( Appt& a : user_appointments ) {
        if ( a.begin == beginstr ) {
            temp_vector.push_back( a );
        }
    }

    // check if the user has any appointments
    if ( !temp_vector.empty() ) {
        // print out the appointments
        std::cout << std::endl << "Appointments found for ";
        rlutil::saveDefaultColor();
        rlutil::setColor(rlutil::GREEN);
        std::cout << beginstr << std::endl;
        rlutil::resetColor();

        rlutil::saveDefaultColor();
        rlutil::setColor(rlutil::CYAN);
        for (Appt a : temp_vector) {
            //std::cout << user_appointments[i];
            a.operator<<(std::cout);
        }
        rlutil::resetColor();
    }
    else {
        // the user doesn't have any appointments
        rlutil::saveDefaultColor();
        rlutil::setColor(rlutil::RED);
        std::cout << std::endl << "No Appointments Found for " << beginstr << std::endl << std::endl;
        rlutil::resetColor();
    }
}

int app::viewApptsByRange() {
    int hh, mm, MM, DD, YYYY;
    std::cout << std::endl << "Begin Time (hh:mm MM/DD/YYYY 24-hour model)" << std::endl;

    do {
        std::cout << "hh: ";
        std::cin >> hh;
    } while ( !ERRCHECKER::HOUR(hh) );

    do {
        std::cout << "mm: ";
        std::cin >> mm;
    } while ( !ERRCHECKER::MINUTE(mm) );

    do {
        std::cout << "MM: ";
        std::cin >> MM;
    } while ( !ERRCHECKER::MONTH(MM) );

    do {
        std::cout << "DD: ";
        std::cin >> DD;
    } while ( !ERRCHECKER::DAY(DD) );

    do {
        std::cout << "YYYY: ";
        std::cin >> YYYY;
    } while ( !ERRCHECKER::YEAR(YYYY) );

    std::string _hh, _mm, _MM, _DD, _YYYY;
    _hh = std::to_string(hh);
    if ( hh == 0 ) {
        _hh = "00";
    }
    else if ( hh < 10 ) {
        _hh = "0" + _hh;
    }

    _mm = std::to_string(mm);
    if ( mm == 0 ) {
        _mm = "00";
    }
    else if ( mm < 10 ) {
        _mm = "0" + _hh;
    }

    _MM = std::to_string(MM);
    if ( MM < 10 ) {
        _MM = "0" + _MM;
    }

    _DD = std::to_string(DD);
    if ( DD < 10 ) {
        _DD = "0" + _DD;
    }

    _YYYY = std::to_string(YYYY);

    std::string beginstr = _hh + ":" + _mm + " " + _MM + "/" + _DD + "/" + _YYYY;

    std::cout << std::endl << "End Time (hh:mm MM/DD/YYYY 24-hour model)" << std::endl;

    do {
        std::cout << "hh: ";
        std::cin >> hh;
    } while ( !ERRCHECKER::HOUR(hh) );

    do {
        std::cout << "mm: ";
        std::cin >> mm;
    } while ( !ERRCHECKER::MINUTE(mm) );

    do {
        std::cout << "MM: ";
        std::cin >> MM;
    } while ( !ERRCHECKER::MONTH(MM) );

    do {
        std::cout << "DD: ";
        std::cin >> DD;
    } while ( !ERRCHECKER::DAY(DD) );

    do {
        std::cout << "YYYY: ";
        std::cin >> YYYY;
    } while ( !ERRCHECKER::YEAR(YYYY) );

    _hh = std::to_string(hh);
    if ( hh == 0 ) {
        _hh = "00";
    }
    else if ( hh < 10 ) {
        _hh = "0" + _hh;
    }

    _mm = std::to_string(mm);
    if ( mm == 0 ) {
        _mm = "00";
    }
    else if ( mm < 10 ) {
        _mm = "0" + _hh;
    }

    _MM = std::to_string(MM);
    if ( MM < 10 ) {
        _MM = "0" + _MM;
    }

    _DD = std::to_string(DD);
    if ( DD < 10 ) {
        _DD = "0" + _DD;
    }

    _YYYY = std::to_string(YYYY);

    std::string endstr = _hh + ":" + _mm + " " + _MM + "/" + _DD + "/" + _YYYY;

    Appt range_appt;
    range_appt.begin = beginstr;
    range_appt.end = endstr;

    // Get the range of appointments ---------------------------------------------------------------------
    std::vector<Appt> temp_vector;
    for ( Appt& a : user_appointments ) {

        // If the appt starts before the range, but ends after the beginning of the range
        if ( Appt::timeLessThan(a.begin, beginstr) && Appt::timeLessThan(beginstr, a.end) ) {
            temp_vector.push_back( a );
        }

        // If the appt starts after the range begins, but before the end of the range end
        if ( Appt::timeLessThan(beginstr, a.begin) && Appt::timeLessThan(a.end, endstr) ) {
            temp_vector.push_back( a );
        }

    }

    // Print the appointments ----------------------------------------------------------------------------
    // check if the user has any appointments
    if ( !temp_vector.empty() ) {
        // print out the appointments
        std::cout << std::endl << "Appointments scheduled between ";
        rlutil::saveDefaultColor();
        rlutil::setColor(rlutil::GREEN);
        std::cout << beginstr;
        rlutil::resetColor();
        std::cout << " and ";
        rlutil::saveDefaultColor();
        rlutil::setColor(rlutil::GREEN);
        std::cout << endstr << std::endl;
        rlutil::resetColor();


        rlutil::saveDefaultColor();
        for (Appt a : temp_vector) {
            rlutil::saveDefaultColor();
            rlutil::setColor(rlutil::CYAN);
            a.operator<<(std::cout);
            rlutil::resetColor();
        }
    }
    else {
        // the user doesn't have any appointments
        rlutil::saveDefaultColor();
        rlutil::setColor(rlutil::RED);
        std::cout << std::endl << "No Appointments Scheduled" << std::endl << std::endl;
        rlutil::resetColor();
    }
}

int app::addAppt() {

    // send control byte
    if ((send(sockfd, &C_ADD_APPT_BIN, sizeof(int), 0)) == -1) {
        int e = errno;
        std::cout << "errno -> " << e;
        return APP_RESPONSE::SEND;
    }

    int hh, mm, MM, DD, YYYY;
    std::cout << std::endl << "Begin Time (hh:mm MM/DD/YYYY 24-hour model)" << std::endl;

    do {
        std::cout << "hh: ";
        std::cin >> hh;
    } while ( !ERRCHECKER::HOUR(hh) );

    do {
        std::cout << "mm: ";
        std::cin >> mm;
    } while ( !ERRCHECKER::MINUTE(mm) );

    do {
        std::cout << "MM: ";
        std::cin >> MM;
    } while ( !ERRCHECKER::MONTH(MM) );

    do {
        std::cout << "DD: ";
        std::cin >> DD;
    } while ( !ERRCHECKER::DAY(DD) );

    do {
        std::cout << "YYYY: ";
        std::cin >> YYYY;
    } while ( !ERRCHECKER::YEAR(YYYY) );

    std::string _hh, _mm, _MM, _DD, _YYYY;
    _hh = std::to_string(hh);
    if ( hh == 0 ) {
        _hh = "00";
    }
    else if ( hh < 10 ) {
        _hh = "0" + _hh;
    }

    _mm = std::to_string(mm);
    if ( mm == 0 ) {
        _mm = "00";
    }
    else if ( mm < 10 ) {
        _mm = "0" + _hh;
    }

    _MM = std::to_string(MM);
    if ( MM < 10 ) {
        _MM = "0" + _MM;
    }

    _DD = std::to_string(DD);
    if ( DD < 10 ) {
        _DD = "0" + _DD;
    }

    _YYYY = std::to_string(YYYY);

    std::string beginstr = _hh + ":" + _mm + " " + _MM + "/" + _DD + "/" + _YYYY;

    std::cout << std::endl << "End Time (hh:mm MM/DD/YYYY 24-hour model)" << std::endl;
    do {
        std::cout << "hh: ";
        std::cin >> hh;
    } while ( !ERRCHECKER::HOUR(hh) );

    do {
        std::cout << "mm: ";
        std::cin >> mm;
    } while ( !ERRCHECKER::MINUTE(mm) );

    do {
        std::cout << "MM: ";
        std::cin >> MM;
    } while ( !ERRCHECKER::MONTH(MM) );

    do {
        std::cout << "DD: ";
        std::cin >> DD;
    } while ( !ERRCHECKER::DAY(DD) );

    do {
        std::cout << "YYYY: ";
        std::cin >> YYYY;
    } while ( !ERRCHECKER::YEAR(YYYY) );

    std::cin.ignore();

    _hh = std::to_string(hh);
    if ( hh == 0 ) {
        _hh = "00";
    }
    else if ( hh < 10 ) {
        _hh = "0" + _hh;
    }

    _mm = std::to_string(mm);
    if ( mm == 0 ) {
        _mm = "00";
    }
    else if ( mm < 10 ) {
        _mm = "0" + _hh;
    }

    _MM = std::to_string(MM);
    if ( MM < 10 ) {
        _MM = "0" + _MM;
    }

    _DD = std::to_string(DD);
    if ( DD < 10 ) {
        _DD = "0" + _DD;
    }

    _YYYY = std::to_string(YYYY);

    std::string endstr = _hh + ":" + _mm + " " + _MM + "/" + _DD + "/" + _YYYY;

    std::string place;
    do {
        std::cout << std::endl << "Place ( " << APPT_PLACE_LENGTH << " characters or fewer ): ";
        getline(std::cin, place);
    } while ( !Appt::PLACE( place ) );

    std::string contents;
    do {
        std::cout << std::endl << "Description ( " << APPT_CONTENTS_LENGTH << " characters or fewer ): ";
        getline(std::cin, contents);
    } while ( !Appt::CONTENTS( contents ) );

    // ----------------------------------------------------------------------------------------------------------------------
    // send the size
    int s = beginstr.length() + 1;
    if ( ( send( sockfd, &s, sizeof(int), 0 ) ) == -1 ) {
        return APP_RESPONSE::SEND;
    }
    // send the begin time
    if ( ( send( sockfd, beginstr.c_str(), s, 0 ) ) == -1 ) {
        return APP_RESPONSE::SEND;
    }

    // ----------------------------------------------------------------------------------------------------------------------
    // send the size
    s = endstr.length() + 1;
    if ( ( send( sockfd, &s, sizeof(int), 0 ) ) == -1 ) {
        return APP_RESPONSE::SEND;
    }
    // send the end time
    if ( ( send( sockfd, endstr.c_str(), s, 0 ) ) == -1 ) {
        return APP_RESPONSE::SEND;
    }

    // ----------------------------------------------------------------------------------------------------------------------
    // send the size
    s = place.length() + 1;
    if ( ( send( sockfd, &s, sizeof(int), 0 ) ) == -1 ) {
        return APP_RESPONSE::SEND;
    }
    // send the place
    if ((send(sockfd, place.c_str(), s, 0)) == -1) {
        return APP_RESPONSE::SEND;
    }

    // ----------------------------------------------------------------------------------------------------------------------
    // send the size
    s = contents.length() + 1;
    if ( ( send( sockfd, &s, sizeof(int), 0 ) ) == -1 ) {
        return APP_RESPONSE::SEND;
    }
    // send the contents
    if ((send(sockfd, contents.c_str(), s, 0)) == -1) {
        return APP_RESPONSE::SEND;
    }

    // response values
    // 0000 0001 (1) – OK
    // 0000 0010 (2) - ERR
    // 0000 0011 (3) – Malformed Data
    const int OK = 0b0001;
    const int ERR = 0b0010;
    const int MAL = 0b0011;
    int re;
    int numbytes = recv( sockfd, &re, sizeof(int), 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::DISCONN; }

    switch (re) {
        case 1:
            rlutil::saveDefaultColor();
            rlutil::setColor(rlutil::GREEN);
            std::cout << std::endl << "Appointment added successfully" << std::endl;
            rlutil::resetColor();
            break;
        case 2:
            rlutil::saveDefaultColor();
            rlutil::setColor(rlutil::RED);
            std::cout << std::endl << "Server error. Please try again." << std::endl;
            rlutil::resetColor();
            break;
        case 3:
            rlutil::saveDefaultColor();
            rlutil::setColor(rlutil::RED);
            std::cout << std::endl << "Invalid Data. Please try again" << std::endl;
            rlutil::resetColor();
        default: break;

    }

    return -1;
}

int app::delAppt() {
    // send control byte
    if ((send(sockfd, &C_DEL_APPT_BIN, sizeof(int), 0)) == -1) {
        return APP_RESPONSE::SEND;
    }

    int id;
    std::cout << std::endl << "Appointment ID:";
    std::cin >> id;

    // send control byte
    if ((send(sockfd, &id, sizeof(int), 0)) == -1) { return APP_RESPONSE::SEND; }

    // response values
    // 0000 0001 (1) – OK
    // 0000 0010 (2) - ERR
    // 0000 0011 (3) – NOEXIST
    // 0000 0100 (4) - MALFORMED DATA
    const int OK = 0b0001;
    const int ERR = 0b0010;
    const int NOEXIST = 0b0011;
    const int MAL = 0b0100;

    int re;
    int numbytes = recv( sockfd, &re, sizeof(int), 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::DISCONN; }

    switch (re) {
        case OK:
            rlutil::saveDefaultColor();
            rlutil::setColor(rlutil::GREEN);
            std::cout << std::endl << "Appointment deleted successfully" << std::endl << std::endl;
            rlutil::resetColor();
            break;
        case NOEXIST:
            rlutil::saveDefaultColor();
            rlutil::setColor(rlutil::RED);
            std::cout << std::endl << "Appointment not found. Please try again" << std::endl << std::endl;
            rlutil::resetColor();
            break;
        case ERR:
            rlutil::saveDefaultColor();
            rlutil::setColor(rlutil::RED);
            std::cout << std::endl << "Server error. Please try again" << std::endl << std::endl;
            rlutil::resetColor();
            break;
        case MAL:
            rlutil::saveDefaultColor();
            rlutil::setColor(rlutil::RED);
            std::cout << std::endl << "Invalid data. Please try again" << std::endl << std::endl;
            rlutil::resetColor();
            break;
        default: break;
    }

    return -1;
}

int app::updateAppt() {
    user_appointments.clear();
    viewAppts();

    // send control byte
    if ((send(sockfd, &C_UPDATE_APPT_BIN, sizeof(int), 0)) == -1) {
        return APP_RESPONSE::SEND;
    }

    int id;
    std::cout << "Appointment ID: ";
    std::cin >> id;

    std::cout << std::endl << "Appointment to update" << std::endl;

    rlutil::saveDefaultColor();
    rlutil::setColor(rlutil::GREEN);
    for (Appt a : user_appointments) {
        if ( a.ID == id ) {
            a.operator<<(std::cout);
            break;
        }
    }

    rlutil::resetColor();

    int hh, mm, MM, DD, YYYY;
    std::cout << std::endl << "Begin Time (hh:mm MM/DD/YYYY 24-hour model)" << std::endl;

    do {
        std::cout << "hh: ";
        std::cin >> hh;
    } while ( !ERRCHECKER::HOUR(hh) );

    do {
        std::cout << "mm: ";
        std::cin >> mm;
    } while ( !ERRCHECKER::MINUTE(mm) );

    do {
        std::cout << "MM: ";
        std::cin >> MM;
    } while ( !ERRCHECKER::MONTH(MM) );

    do {
        std::cout << "DD: ";
        std::cin >> DD;
    } while ( !ERRCHECKER::DAY(DD) );

    do {
        std::cout << "YYYY: ";
        std::cin >> YYYY;
    } while ( !ERRCHECKER::YEAR(YYYY) );

    std::string _hh, _mm, _MM, _DD, _YYYY;
    _hh = std::to_string(hh);
    if ( hh == 0 ) {
        _hh = "00";
    }
    else if ( hh < 10 ) {
        _hh = "0" + _hh;
    }

    _mm = std::to_string(mm);
    if ( mm == 0 ) {
        _mm = "00";
    }
    else if ( mm < 10 ) {
        _mm = "0" + _hh;
    }

    _MM = std::to_string(MM);
    if ( MM < 10 ) {
        _MM = "0" + _MM;
    }

    _DD = std::to_string(DD);
    if ( DD < 10 ) {
        _DD = "0" + _DD;
    }

    _YYYY = std::to_string(YYYY);

    std::string beginstr = _hh + ":" + _mm + " " + _MM + "/" + _DD + "/" + _YYYY;

    std::cout << std::endl << "End Time (hh:mm MM/DD/YYYY 24-hour model)" << std::endl;
    do {
        std::cout << "hh: ";
        std::cin >> hh;
    } while ( !ERRCHECKER::HOUR(hh) );

    do {
        std::cout << "mm: ";
        std::cin >> mm;
    } while ( !ERRCHECKER::MINUTE(mm) );

    do {
        std::cout << "MM: ";
        std::cin >> MM;
    } while ( !ERRCHECKER::MONTH(MM) );

    do {
        std::cout << "DD: ";
        std::cin >> DD;
    } while ( !ERRCHECKER::DAY(DD) );

    do {
        std::cout << "YYYY: ";
        std::cin >> YYYY;
    } while ( !ERRCHECKER::YEAR(YYYY) );

    std::cin.ignore();

    _hh = std::to_string(hh);
    if ( hh == 0 ) {
        _hh = "00";
    }
    else if ( hh < 10 ) {
        _hh = "0" + _hh;
    }

    _mm = std::to_string(mm);
    if ( mm == 0 ) {
        _mm = "00";
    }
    else if ( mm < 10 ) {
        _mm = "0" + _hh;
    }

    _MM = std::to_string(MM);
    if ( MM < 10 ) {
        _MM = "0" + _MM;
    }

    _DD = std::to_string(DD);
    if ( DD < 10 ) {
        _DD = "0" + _DD;
    }

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
    // send the appt id
    if ( ( send( sockfd, &id, sizeof(int), 0 ) ) == -1 ) {
        return APP_RESPONSE::SEND;
    }

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


    // response values
    // 0000 0001 (1) – OK
    // 0000 0010 (2) - ERR
    // 0000 0011 (3) - NOTFOUND
    // 0000 0100 (4) – Malformed Data
    const int OK = 0b0001;
    const int ERR = 0b0010;
    const int NOTFOUND = 0b0011;
    const int MAL = 0b0100;

    int re;
    int numbytes = recv( sockfd, &re, sizeof(int), 0 );
    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) { return APP_RESPONSE::DISCONN; }

    switch (re) {
        case OK:
            rlutil::saveDefaultColor();
            rlutil::setColor(rlutil::GREEN);
            std::cout << std::endl << "Appointment updated successfully" << std::endl;
            rlutil::resetColor();
            break;
        case ERR:
            rlutil::saveDefaultColor();
            rlutil::setColor(rlutil::RED);
            std::cout << std::endl << "Server error. Please try again." << std::endl;
            rlutil::resetColor();
            break;
        case NOTFOUND:
            rlutil::saveDefaultColor();
            rlutil::setColor(rlutil::RED);
            std::cout << std::endl << "Appointment not found. Please try again" << std::endl;
            rlutil::resetColor();
            break;
        case MAL:
            rlutil::saveDefaultColor();
            rlutil::setColor(rlutil::RED);
            std::cout << std::endl << "Invalid Data. Please try again" << std::endl;
            rlutil::resetColor();
        default: break;

    }

    return -1;
}

int app::getUserInfo() {
    // send control byte
    if ((send(sockfd, &C_GET_USER_DATA_BIN, sizeof(int), 0)) == -1) {
        return APP_RESPONSE::SEND;
    }

    int numbytes;
    int data_length;
    char passbuf[UserData::MAX_PASSWORD_LENGTH + 1] = {0, };
    char namebuf[UserData::MAX_NAME_LENGTH + 1] = {0, };
    char emailbuf[UserData::MAX_EMAIL_LENGTH + 1] = {0, };
    char phonebuf[UserData::MAX_PHONE_LENGTH + 1] = {0, };

    //------------------------------------------------------------------------
    // get the appt contents
    numbytes = recv( sockfd, &data_length, sizeof(int), 0 );
    //data_length = ntohl(data_length);

    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) {
        return APP_RESPONSE::DISCONN;
    }
    assert(numbytes==sizeof(int));

    numbytes = recv( sockfd, passbuf, data_length, 0 );

    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) {
        return APP_RESPONSE::DISCONN;
    }
    assert(numbytes==data_length);
    //------------------------------------------------------------------------

    //------------------------------------------------------------------------
    // get the appt contents
    numbytes = recv( sockfd, &data_length, sizeof(int), 0 );
    //data_length = ntohl(data_length);

    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) {
        return APP_RESPONSE::DISCONN;
    }
    assert(numbytes==sizeof(int));

    numbytes = recv( sockfd, namebuf, data_length, 0 );

    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) {
        return APP_RESPONSE::DISCONN;
    }
    assert(numbytes==data_length);
    //------------------------------------------------------------------------

    //------------------------------------------------------------------------
    // get the appt contents
    numbytes = recv( sockfd, &data_length, sizeof(int), 0 );
    //data_length = ntohl(data_length);

    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) {
        return APP_RESPONSE::DISCONN;
    }
    assert(numbytes==sizeof(int));

    numbytes = recv( sockfd, emailbuf, data_length, 0 );

    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) {
        return APP_RESPONSE::DISCONN;
    }
    assert(numbytes==data_length);
    //------------------------------------------------------------------------

    //------------------------------------------------------------------------
    // get the appt contents
    numbytes = recv( sockfd, &data_length, sizeof(int), 0 );
    //data_length = ntohl(data_length);

    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) {
        return APP_RESPONSE::DISCONN;
    }
    assert(numbytes==sizeof(int));

    numbytes = recv( sockfd, phonebuf, data_length, 0 );

    if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
    else if ( numbytes == 0 ) {
        return APP_RESPONSE::DISCONN;
    }
    assert(numbytes==data_length);
    //------------------------------------------------------------------------

    userdata.password = passbuf;
    userdata.name = namebuf;
    userdata.email = emailbuf;
    userdata.phone = phonebuf;

    return -1;
}

int app::updateUserInfo() {
    if ( int resp = getUserInfo() != -1 ) {
        return resp;
    }

    // SEC control
    const int C_UP_PASS = 0b0001;
    const int C_UP_NAME = 0b0010;
    const int C_UP_EMAIL = 0b0011;
    const int C_UP_PHONE = 0b0100;

    int choice;
    do {
        std::cout << std::endl << "Your Information. Choose an item to update." << std::endl;
        rlutil::saveDefaultColor();
        rlutil::setColor(rlutil::CYAN);
        std::cout << userdata << std::endl;
        rlutil::resetColor();
        std::cout << "5. Cancel" << std::endl;
        std::cout << ">";

        std::cin >> choice;


        std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

        if ( choice == 5 ) {
            return -1;
        }

        if ((send(sockfd, &C_UPDATE_USER_DATA_BIN, sizeof(int), 0)) == -1) {
            return APP_RESPONSE::SEND;
        }

        std::string temp_pass, pass, name, email, phone, sender;

        int c_val;

        if ( choice == 1)
        {
            c_val = C_UP_PASS;
            do {
                std::cout << "New Password (8-16 characters): ";
                getline(std::cin, pass);
            } while ( !UserData::PASSWORD(pass) );

            if (UserData::PASSWORD(pass)) {
                temp_pass = pass;
                sender = pass;
                for (char &c : temp_pass) { c = '*'; }
                userdata.password = temp_pass;
                encrypt( sender, (uint)sender.length() );
            }
        }
        else if ( choice == 2 )
        {
            c_val = C_UP_NAME;
            do {
                std::cout << "New Name (Up to 50 characters): ";
                getline(std::cin, name);
            } while ( !UserData::NAME( name ) );
            userdata.name = name;
            sender = name;
        }
        else if ( choice == 3 )
        {
            c_val = C_UP_EMAIL;
            do {
                std::cout << "New email (Up to 100 characters): ";
                getline(std::cin, email);
            } while ( !UserData::EMAIL( email ) );
            userdata.email = email;
            sender = email;
        }
        else if ( choice == 4 )
        {
            c_val = C_UP_PHONE;
            do {
                std::cout << "New Phone (###-###-####): ";
                getline(std::cin, phone);
            } while ( !UserData::PHONE( phone ) );
            userdata.phone = phone;
            sender = phone;
        }

        if ((send(sockfd, &c_val, sizeof(int), 0)) == -1) {
            return APP_RESPONSE::SEND;
        }

        int data_size = sender.length() + 1;
        if ((send(sockfd, &data_size, sizeof(int), 0)) == -1) {
            return APP_RESPONSE::SEND;
        }

        if ((send(sockfd, sender.c_str(), data_size, 0)) == -1) {
            return APP_RESPONSE::SEND;
        }

        // response values
        // 0000 0001 (1) – OK
        // 0000 0010 (2) - ERR
        // 0000 0011 (3) – NOEXIST
        // 0000 0100 (4) - MALFORMED DATA
        const int OK = 0b0001;
        const int ERR = 0b0010;
        const int MAL = 0b0011;

        int re;
        int numbytes = recv( sockfd, &re, sizeof(int), 0 );
        if ( numbytes == -1 ) { return APP_RESPONSE::RECV; }
        else if ( numbytes == 0 ) { return APP_RESPONSE::DISCONN; }

        switch (re) {
            case OK:
                rlutil::saveDefaultColor();
                rlutil::setColor(rlutil::GREEN);
                std::cout << std::endl << "Information updated." << std::endl << std::endl;
                rlutil::resetColor();
                break;
            case ERR:
                rlutil::saveDefaultColor();
                rlutil::setColor(rlutil::RED);
                std::cout << std::endl << "Server error. Please try again" << std::endl << std::endl;
                rlutil::resetColor();
                break;
            case MAL:
                rlutil::saveDefaultColor();
                rlutil::setColor(rlutil::RED);
                std::cout << std::endl << "Invalid data. Please try again" << std::endl << std::endl;
                rlutil::resetColor();
                break;
            default: break;
        }

    } while ( choice != 5 );

    return -1;
}

void app::encrypt(char s[], uint len ) {
    for ( int i = 0; i < len; ++i ) {
        s[i] = s[i] + 13;
    }
}

void app::encrypt(std::string& s, uint len ) {
    for ( int i = 0; i < len; ++i ) {
        s[i] = s[i] + 13;
    }
}