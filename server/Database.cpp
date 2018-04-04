#include <sys/stat.h>
#include "Database.h"

//line.replace(line.find(deleteline),deleteline.length(),"");
//temp << line << endl;

int Database::addUser( const std::string& uname, const std::string& pass ) {
    int stat;
    std::string dir = DB_DIRNAME + uname;
    stat = mkdir( dir.c_str(), 0755 );

    if ( stat == 0 ) {
        dir = dir + "\\";
        user_dir.open( dir + f_user, std::fstream::out );
        if ( !user_dir ) {
            return -1;
        }
        user_dir << pass << '\n';
        user_dir.close();

        user_dir.open( dir + f_appointments, std::fstream::out );
        if ( !user_dir ) {
            return -1;
        }
        user_dir.close();
    }

    return stat;
}