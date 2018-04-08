#ifndef SERVER_USERDATA_H
#define SERVER_USERDATA_H

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
#endif //SERVER_USERDATA_H
