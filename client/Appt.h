#ifndef CLIENT_APPT_H
#define CLIENT_APPT_H

#include <string>
#include <iostream>
static const int APPT_ID_LENGTH = 10;
static const int APPT_BEGIN_END_LENGTH = 16;
static const int APPT_PLACE_LENGTH = 50;
static const int APPT_CONTENTS_LENGTH = 256;

struct Appt {
    int ID = 0;
    std::string begin = "";
    std::string end = "";
    std::string place = "";
    std::string contents = "";

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
