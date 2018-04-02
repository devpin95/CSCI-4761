#ifndef CLIENT_CONTROL_PROTOCOL_CONSTS_H
#define CLIENT_CONTROL_PROTOCOL_CONSTS_H

#include <string>

static const int MAXCONTROLSIZE = 1;

// Data Formats
static const int MAX_USERNAME_LENGTH = 21;
static const int MIN_PASSWORD_LENGTH = 8;
static const int MAX_PASSWORD_LENGTH = 16;

// Control Values (bytes)
static const int C_LOGIN_BIN = 0b0001;
static const int C_ADD_USER_BIN = 0b0010;

// Control Values (std::string)
static const std::string C_LOGIN = std::to_string(C_LOGIN_BIN);
static const std::string C_ADD_USER = std::to_string(C_ADD_USER_BIN);

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
        return pass.length() < MIN_PASSWORD_LENGTH || pass.length() > MAX_PASSWORD_LENGTH;
    }
};

#endif //CLIENT_CONTROL_PROTOCOL_CONSTS_H
