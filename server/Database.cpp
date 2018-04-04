#include "Database.h"

int Database::checkIfUserExists(const std::string &uname) {
    int timeout_threshold = 100;
    int timeout_counter = 0;

    user_file.open( user_file_path, std::fstream::in );
    while ( !user_file ) {
        ++timeout_counter;

        if ( timeout_counter > timeout_threshold ) {
            return -1;
        }

        user_file.open( user_file_path, std::fstream::in );
    }

//    if (!user_file) {
//        std::cerr << "Unable to open file datafile.txt";
//        return -1;
//    }

    std::string u;
    while ( user_file >> u && !u.empty() ) {
        if ( u == uname ) {
            return 0;
        }
        user_file >> u;
    }

    user_file.close();
    return 1;
}

int Database::addUser( const std::string& uname, const std::string& pass ) {
    int timeout_threshold = 100;
    int timeout_counter = 0;

    user_file.open( user_file_path, std::ios::app );
    while ( !user_file ) {
        ++timeout_counter;

        if ( timeout_counter > timeout_threshold ) {
            return -1;
        }

        user_file.open( user_file_path, std::fstream::app );
    }

//    if (!user_file) {
//        std::cerr << "Unable to open file datafile.txt";
//        return -1;
//    }

    user_file << uname << "\n";
    user_file << pass << "\n";
    user_file.close();

    return 1;
}