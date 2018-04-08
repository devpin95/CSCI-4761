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

    const int apptBeginHour() {
        std::string temp_str;
        temp_str += begin[0];
        temp_str += begin[1];
        return stoi(temp_str);
    }
    const int apptEndHour() {
        std::string temp_str;
        temp_str += end[0];
        temp_str += end[1];
        return stoi(temp_str);
    }

    const int apptBeginMinute() {
        std::string temp_str;
        temp_str += begin[3];
        temp_str += begin[4];
        return stoi(temp_str);
    }
    const int apptEndMinute() {
        std::string temp_str;
        temp_str += end[3];
        temp_str += end[4];
        return stoi(temp_str);
    }

    const int apptBeginMonth() {
        std::string temp_str;
        temp_str += begin[6];
        temp_str += begin[7];
        return stoi(temp_str);
    }
    const int apptEndMonth() {
        std::string temp_str;
        temp_str += end[6];
        temp_str += end[7];
        return stoi(temp_str);
    }

    const int apptBeginDay() {
        std::string temp_str;
        temp_str += begin[9];
        temp_str += begin[10];
        return stoi(temp_str);
    }
    const int apptEndDay() {
        std::string temp_str;
        temp_str += end[9];
        temp_str += end[10];
        return stoi(temp_str);
    }

    const int apptBeginYear() {
        std::string temp_str;
        temp_str += begin[12];
        temp_str += begin[13];
        temp_str += begin[14];
        temp_str += begin[15];
        return stoi(temp_str);
    }
    const int apptEndYear() {
        std::string temp_str;
        temp_str += begin[12];
        temp_str += begin[13];
        temp_str += begin[14];
        temp_str += begin[15];
        return stoi(temp_str);
    }

    bool operator<( Appt& b ) {
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
};

#endif //CLIENT_APPT_H
