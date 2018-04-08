#include "Database.h"

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

int Database::login(const std::string &uname, const std::string &pass) {
    int stat = 0;
    std::string file_pass;
    std::string dir = DB_DIRNAME + uname + "\\" + f_user;
    user_dir.open(dir);

    if ( user_dir ) {
        user_dir >> file_pass;
        user_dir.close();

        if ( pass != file_pass ) {
            stat = -1;
        } else {
            user_dir_path = DB_DIRNAME + uname + "\\";
            logged_in = true;
        }
    } else {
        stat = -1;
    }

    return stat;
}

int Database::getAppts(std::vector<Appt> &a) {
    if ( !logged_in ) {
        return -1;
    }
    int size = 0;
    std::string ID, begin, end, place, contents;

    //rlutil::saveDefaultColor();
    //rlutil::setColor(rlutil::YELLOW);
    user_dir.open( user_dir_path  + "\\" + f_appointments, std::ios::in );
    //rlutil::resetColor();

    if ( user_dir ) {
        while ( getline( user_dir, ID) ) {
            if ( !ID.empty() ) {
                getline( user_dir, begin);
                getline( user_dir, end);
                getline( user_dir, place);
                getline( user_dir, contents);

                Appt appt;
                appt.ID = stoi(ID);
                appt.begin = begin;
                appt.end = end;
                appt.place = place;
                appt.contents = contents;

                a.push_back(appt);
                ++size;
            }
        }
        user_dir.close();

    } else {
        return -1;
    }

    return size;
}

int Database::addAppt( const std::string& begin, const std::string& end, const std::string& place, const std::string& contents ) {
    if ( !logged_in ) {
        return -1;
    }

    int max_id = 0;
    std::string ID, b, e, p, c;
    std::vector<Appt> temp_appts;

    //rlutil::saveDefaultColor();
    //rlutil::setColor(rlutil::YELLOW);
    std::cout << user_dir_path << "\\" << f_appointments;
    //rlutil::resetColor();

    user_dir.open( user_dir_path  + "\\" + f_appointments, std::ios::in );

    if ( user_dir ) {
        while ( getline( user_dir, ID) ) {
            if ( !ID.empty() ) {
                max_id = std::max(max_id, stoi(ID));

                getline( user_dir, b);
                getline( user_dir, e);
                getline( user_dir, p);
                getline( user_dir, c);

                Appt appt;
                appt.ID = stoi(ID);
                appt.begin = b;
                appt.end = e;
                appt.place = p;
                appt.contents = c;

                temp_appts.push_back(appt);
            }
        }
        user_dir.close();

        Appt new_appt;
        new_appt.ID = max_id + 1;
        new_appt.begin = begin;
        new_appt.end = end;
        new_appt.place = place;
        new_appt.contents = contents;

        temp_appts.push_back( new_appt );

    }
    else {
        return -1;
    }

    user_dir.open( user_dir_path  + "\\" + f_appointments, std::ios::out | std::ios::trunc );

    if ( user_dir ) {
        for ( Appt& a : temp_appts ) {
            user_dir << a;
        }
    }
    else {
        return -1;
    }

    user_dir.close();

    return 0;
}

int Database::delAppt(const int& id) {
    if ( !logged_in ) {
        return -1;
    }

    int max_id = 0;
    std::string ID, b, e, p, c;
    std::vector<Appt> temp_appts;

    user_dir.open( user_dir_path  + "\\" + f_appointments, std::ios::in );

    if ( user_dir ) {
        while ( getline( user_dir, ID) ) {
            if ( !ID.empty() ) {
                max_id = std::max(max_id, stoi(ID));

                getline( user_dir, b);
                getline( user_dir, e);
                getline( user_dir, p);
                getline( user_dir, c);

                Appt appt;
                appt.ID = stoi(ID);
                appt.begin = b;
                appt.end = e;
                appt.place = p;
                appt.contents = c;

                temp_appts.push_back(appt);
            }
        }
        user_dir.close();

        for ( int i = 0; i < temp_appts.size(); ++i ) {
            if ( temp_appts[i].ID == id ) {
                temp_appts.erase(temp_appts.begin()+i);
                break;
            }
        }

        user_dir.open( user_dir_path  + "\\" + f_appointments, std::ios::out | std::ios::trunc );
        if ( user_dir ) {
            for ( Appt& a : temp_appts ) {
                user_dir << a;
            }
        } else {
            return -1;
        }

        user_dir.close();
    }
    else {
        return -1;
    }

    return 0;
}

int Database::updateAppt(const int& id,const std::string& begin, const std::string& end, const std::string& place, const std::string& contents) {
    if ( !logged_in ) {
        return -1;
    }

    std::string ID, b, e, p, c;
    std::vector<Appt> temp_appts;

    user_dir.open( user_dir_path  + "\\" + f_appointments, std::ios::in );

    if ( user_dir ) {
        while ( getline( user_dir, ID) ) {
            if ( !ID.empty() ) {
                getline( user_dir, b);
                getline( user_dir, e);
                getline( user_dir, p);
                getline( user_dir, c);

                Appt appt;
                appt.ID = stoi(ID);
                appt.begin = b;
                appt.end = e;
                appt.place = p;
                appt.contents = c;

                temp_appts.push_back(appt);
            }
        }
        user_dir.close();

        bool appt_updated = false;
        for ( int i = 0; i < temp_appts.size(); ++i ) {
            if ( temp_appts[i].ID == id ) {
                temp_appts[i].begin = begin;
                temp_appts[i].end = end;
                temp_appts[i].place = place;
                temp_appts[i].contents = contents;
                appt_updated = true;
                break;
            }
        }

        if ( !appt_updated ) {
            return 1;
        }

        user_dir.open( user_dir_path  + "\\" + f_appointments, std::ios::out | std::ios::trunc );
        if ( user_dir ) {
            for ( Appt& a : temp_appts ) {
                user_dir << a;
            }
        } else {
            return -1;
        }

        user_dir.close();
    }
    else {
        return -1;
    }

    return 0;
}