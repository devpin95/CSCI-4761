#ifndef SERVER_DATABASE_H
#define SERVER_DATABASE_H

#include <string>
#include <fstream>
#include <iostream>

class Database {
public:
    int checkIfUserExists( const std::string& uname );
    int addUser( const std::string& uname, const std::string& pass );

private:
    const std::string user_file_path = "db\\users.txt";
    std::fstream user_file;
};


#endif //SERVER_DATABASE_H
