#ifndef SERVER_DATABASE_H
#define SERVER_DATABASE_H

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <sys/stat.h>
#include "Appt.h"
#include "UserData.h"
//#include "rlutil.h"

class Database {
public:
    int addUser( const std::string& uname, const std::string& pass );
    int login( const std::string& uname, const std::string& pass );
    int getAppts( std::vector<Appt> &a );
    int addAppt( const std::string& begin, const std::string& end, const std::string& place, const std::string& contents );
    int delAppt( const int& id );
    int updateAppt(const int& id,const std::string& begin, const std::string& end, const std::string& place, const std::string& contents);
    int getUserData( UserData& u );
    int updateUserData( UserData& u );

private:
    const std::string f_user = "user";  // stores user information
    const std::string f_appointments = "appointments";  // stores user appointements
    const std::string DB_DIRNAME = "C:/Users/devpin/Documents/School/ComputerNetworks/CSCI-4761/server/db/";
    std::string user_dir_path = DB_DIRNAME;
    int logged_in = false;
    std::fstream user_dir;
};


#endif //SERVER_DATABASE_H
