#ifndef SERVER_USERDATA_H
#define SERVER_USERDATA_H

#include <string>
#include <iostream>
#include <fstream>

struct UserData {
    // Data Formats
    static const int MAX_USERNAME_LENGTH = 21;
    static const int MIN_PASSWORD_LENGTH = 8;
    static const int MAX_PASSWORD_LENGTH = 16;
    static const int MAX_NAME_LENGTH = 50;
    static const int MAX_EMAIL_LENGTH = 100;
    static const int MAX_PHONE_LENGTH = 12;

    std::string password;
    std::string name;
    std::string email;
    std::string phone;

    static const bool USERNAME(const std::string& uname) {

        if ( uname.length() < 0 || uname.length() > MAX_USERNAME_LENGTH ) {
            return false;
        }

        // check if
        for ( auto c : uname ) {
            if ( isalnum( c ) == 0 || c == '-') {
                return false;
            }
        }

        return true;
    }

    static const bool PASSWORD( const std::string& pass ) {
        return pass.length() >= MIN_PASSWORD_LENGTH && pass.length() <= MAX_PASSWORD_LENGTH;
    }

    static const bool NAME(const std::string& name) {
        return name.length() > 0 && name.length() < MAX_NAME_LENGTH;
    }

    static const bool EMAIL( const std::string& email ) {
        return email.length() > 0 && email.length() < MAX_EMAIL_LENGTH;
    }

    static const bool PHONE( const std::string& phone ) {
        return phone.length() == MAX_PHONE_LENGTH;
    }

};

std::fstream& operator<<( std::fstream& out, const UserData& u );
#endif //SERVER_USERDATA_H
