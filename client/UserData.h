//
// Created by devpin on 4/7/2018.
//

#ifndef CLIENT_USERDATA_H
#define CLIENT_USERDATA_H


#include <string>
#include <iostream>
#include <fstream>

struct UserData {
    std::string password;
    std::string name;
    std::string email;
    std::string phone;
};

std::fstream& operator<<( std::fstream& out, const UserData& u );
std::ostream& operator<<( std::ostream& out, const UserData& u );


#endif //CLIENT_USERDATA_H
