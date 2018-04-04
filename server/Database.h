#ifndef SERVER_DATABASE_H
#define SERVER_DATABASE_H

#include <string>
#include <fstream>
#include <iostream>

class Database {
public:
    int addUser( const std::string& uname, const std::string& pass );

private:
    const std::string f_user = "user";  // stores user information
    const std::string f_appointments = "appointments";  // stores user appointements
    const std::string DB_DIRNAME = "C:\\Users\\devpin\\Documents\\School\\ComputerNetworks\\CSCI-4761\\server\\db\\";
    std::fstream user_dir;
};


#endif //SERVER_DATABASE_H
