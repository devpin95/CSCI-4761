#ifndef CLIENT_APPT_H
#define CLIENT_APPT_H

#include <string>
#include <iostream>

static const int APPT_ID_LENGTH = 10;
static const int APPT_BEGIN_END_LENGTH = 16;
static const int APPT_PLACE_LENGTH = 50;
static const int APPT_CONTENTS_LENGTH = 256;

struct Appt {
    int ID = 0;                 //123456789
    std::string begin = "";     //hh:mm MM/DD/YYYY
    std::string end = "";
    std::string place = "";
    std::string contents = "";

    int apptBeginHour() const {
        std::string temp_str;
        temp_str += begin[0];
        temp_str += begin[1];
        return stoi(temp_str);
    }
    int apptEndHour() const {
        std::string temp_str;
        temp_str += end[0];
        temp_str += end[1];
        return stoi(temp_str);
    }

    int apptBeginMinute() const {
        std::string temp_str;
        temp_str += begin[3];
        temp_str += begin[4];
        return stoi(temp_str);
    }
    int apptEndMinute() const {
        std::string temp_str;
        temp_str += end[3];
        temp_str += end[4];
        return stoi(temp_str);
    }

    int apptBeginMonth() const {
        std::string temp_str;
        temp_str += begin[6];
        temp_str += begin[7];
        return stoi(temp_str);
    }
     int apptEndMonth() const {
        std::string temp_str;
        temp_str += end[6];
        temp_str += end[7];
        return stoi(temp_str);
    }

    int apptBeginDay() const {
        std::string temp_str;
        temp_str += begin[9];
        temp_str += begin[10];
        return stoi(temp_str);
    }
    int apptEndDay() const {
        std::string temp_str;
        temp_str += end[9];
        temp_str += end[10];
        return stoi(temp_str);
    }

    int apptBeginYear() const {
        std::string temp_str;
        temp_str += begin[12];
        temp_str += begin[13];
        temp_str += begin[14];
        temp_str += begin[15];
        return stoi(temp_str);
    }
    int apptEndYear() const {
        std::string temp_str;
        temp_str += begin[12];
        temp_str += begin[13];
        temp_str += begin[14];
        temp_str += begin[15];
        return stoi(temp_str);
    }

    bool operator<( const Appt& b ) const {
        if ( apptBeginYear() > b.apptBeginYear() ) {
            return false;
        }
        if ( apptBeginYear() < b.apptBeginYear() ) {
            return true;
        }
        if ( apptBeginMonth() > b.apptBeginMonth() ) {
            return false;
        }
        if ( apptBeginMonth() < b.apptBeginMonth() ) {
            return true;
        }
        if ( apptBeginDay() > b.apptBeginDay() ) {
            return false;
        }
        if ( apptBeginDay() < b.apptBeginDay() ) {
            return true;
        }
        if ( apptBeginHour() > b.apptBeginHour() ) {
            return false;
        }
        if ( apptBeginHour() < b.apptBeginHour() ) {
            return true;
        }
        if ( apptBeginMinute() > b.apptBeginMinute() ) {
            return false;
        }
        if ( apptBeginMinute() < b.apptBeginMinute() ) {
            return true;
        }

        return true;
    }

    static bool timeLessThan( const std::string& b, const std::string& e ) {
        int b_hh, b_mm, b_MM, b_DD, b_YYYY, e_hh, e_mm, e_MM, e_DD, e_YYYY;
        std::string temp_str;

        // Get the hours from both strings
        temp_str += b[0];
        temp_str += b[1];
        b_hh = stoi(temp_str);
        temp_str.clear();
        temp_str += e[0];
        temp_str += e[1];
        e_hh = stoi(temp_str);
        temp_str.clear();

        // Get the minutes from both strings
        temp_str += b[3];
        temp_str += b[4];
        b_mm = stoi(temp_str);
        temp_str.clear();
        temp_str += e[3];
        temp_str += e[4];
        e_mm = stoi(temp_str);
        temp_str.clear();

        // Get the month from both strings
        temp_str += b[6];
        temp_str += b[7];
        b_MM = stoi(temp_str);
        temp_str.clear();
        temp_str += e[6];
        temp_str += e[7];
        e_MM = stoi(temp_str);
        temp_str.clear();

        // Get the day from both strings
        temp_str += b[9];
        temp_str += b[10];
        b_DD = stoi(temp_str);
        temp_str.clear();
        temp_str += e[9];
        temp_str += e[10];
        e_DD = stoi(temp_str);
        temp_str.clear();

        // Get the year from both strings
        temp_str += b[12];
        temp_str += b[13];
        temp_str += b[14];
        temp_str += b[15];
        b_YYYY = stoi(temp_str);
        temp_str.clear();
        temp_str += e[12];
        temp_str += e[13];
        temp_str += e[14];
        temp_str += e[15];
        e_YYYY = stoi(temp_str);
        temp_str.clear();

        if ( b_YYYY > e_YYYY ) {
            return false;
        }
        if ( b_YYYY < e_YYYY ) {
            return true;
        }
        if ( b_MM > e_MM ) {
            return false;
        }
        if ( b_MM < e_MM ) {
            return true;
        }
        if ( b_DD > e_DD ) {
            return false;
        }
        if ( b_DD < e_DD ) {
            return true;
        }
        if ( b_hh > e_hh ) {
            return false;
        }
        if ( b_hh < e_hh ) {
            return true;
        }
        if ( b_mm > e_mm ) {
            return false;
        }
        if ( b_mm < e_mm ) {
            return true;
        }

        return true;

    }

    bool operator==( const Appt& b ) {
        return begin == b.begin && end == b.end && place == b.place && contents == b.contents;
    }

    std::ostream& operator<<(std::ostream& out) {
        out << "-------------------------------------------------" << std::endl;
        out << "| ID: " << ID << std::endl;
        out << "| Begin: " << begin << std::endl;
        out << "| End: " << end << std::endl;
        out << "| Place: " << place << std::endl;
        out << "| Description: " << contents << std::endl;
        out << "-------------------------------------------------" << std::endl << std::endl;
        return out;
    }

    static const bool I(const int& id) {
        return id > 0 && id < INT_MAX;
    }

    static const bool TIME(const std::string& time) {
        return time.length() == APPT_BEGIN_END_LENGTH;
    }

    static const bool PLACE(const std::string& place) {
        return place.length() > 0 && place.length() < APPT_PLACE_LENGTH;
    }

    static const bool CONTENTS(const std::string& contents) {
        return contents.length() > 0 && contents.length() < APPT_CONTENTS_LENGTH;
    }
};

#endif //CLIENT_APPT_H
