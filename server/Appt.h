#ifndef SERVER_APPT_H
#define SERVER_APPT_H

#include <string>
#include <fstream>
static const int APPT_ID_LENGTH = 10;
static const int APPT_BEGIN_END_LENGTH = 17;
static const int APPT_PLACE_LENGTH = 51;
static const int APPT_CONTENTS_LENGTH = 256;

struct Appt {
    int ID = 0;
    std::string begin = "";
    std::string end = "";
    std::string place = "";
    std::string contents = "";

//    std::fstream& operator<<( std::fstream& out ) {
//        out << ID << "\n";
//        out << begin << "\n";
//        out << end << "\n";
//        out << place << "\n";
//        out << contents << std::endl;
//    }
};

std::fstream& operator<<( std::fstream& out, const Appt& a );

#endif //SERVER_APPT_H
