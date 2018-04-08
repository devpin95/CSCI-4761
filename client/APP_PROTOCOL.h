#ifndef CLIENT_CONTROL_PROTOCOL_CONSTS_H
#define CLIENT_CONTROL_PROTOCOL_CONSTS_H

#include <string>

static const int MAXCONTROLSIZE = 1;
static const int MAXCONTROLSIZE_DATA = 2;
static const int MAX_APPTS_SIZE_LENGTH = 10;

// Data Formats
static const int MAX_USERNAME_LENGTH = 21;
static const int MIN_PASSWORD_LENGTH = 8;
static const int MAX_PASSWORD_LENGTH = 16;
static const int MAX_NAME_LENGTH = 50;
static const int MAX_EMAIL_LENGTH = 100;
static const int MAX_PHONE_LENGTH = 12;

// Control Values (bytes)
static const int C_LOGIN_BIN = 0b0001;
static const int C_ADD_USER_BIN = 0b0010;
static const int C_GET_USER_APPTS_BIN = 0b0011;
static const int C_ADD_APPT_BIN = 0b0100;
static const int C_DEL_APPT_BIN = 0b0101;
static const int C_UPDATE_APPT_BIN = 0b0110;
static const int C_GET_USER_DATA_BIN = 0b0111;
static const int C_UPDATE_USER_DATA_BIN = 0b1000;

// Control Values (std::string)
static const std::string C_LOGIN = std::to_string(C_LOGIN_BIN);
static const std::string C_ADD_USER = std::to_string(C_ADD_USER_BIN);
static const std::string C_GET_USER_APPTS = std::to_string(C_GET_USER_APPTS_BIN);
static const std::string C_ADD_APPT = std::to_string(C_ADD_APPT_BIN);
static const std::string C_DEL_APPT = std::to_string(C_DEL_APPT_BIN);
static const std::string C_UPDATE_APPT = std::to_string(C_UPDATE_APPT_BIN);
static const std::string C_GET_USER_DATA = std::to_string(C_GET_USER_DATA_BIN);
static const std::string C_UPDATE_USER_DATA = std::to_string(C_UPDATE_USER_DATA_BIN);

// Error checking suite
struct ERRCHECKER {
    static const bool USERNAME(const std::string& uname) {

        if ( uname.length() < 0 || uname.length() > MAX_USERNAME_LENGTH ) {
            return false;
        }

        // check if
        for ( auto c : uname ) {
            if ( isalnum( c ) == 0 || c == '-' ) {
                return false;
            }
        }

        return true;
    }

    static const bool PASSWORD( const std::string& pass ) {
        return pass.length() >= MIN_PASSWORD_LENGTH && pass.length() <= MAX_PASSWORD_LENGTH;
    }

    static const bool DAY(const int& d) {
        return d > 0 && d <= 31;
    }

    static const bool MONTH(const int& m) {
        return m > 0 && m <= 12;
    }

    static const bool YEAR(const int& y) {
        return y >= 2018  && y <= 3000;
    }

    static const bool HOUR(const int& h) {
        return h >= 0  && h < 24;
    }

    static const bool MINUTE(const int& m) {
        return m >= 0  && m < 60;
    }
};

#endif //CLIENT_CONTROL_PROTOCOL_CONSTS_H
