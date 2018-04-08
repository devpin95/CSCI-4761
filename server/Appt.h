#ifndef SERVER_APPT_H
#define SERVER_APPT_H

#include <string>
#include <fstream>
#include <climits>

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

std::fstream& operator<<( std::fstream& out, const Appt& a );

#endif //SERVER_APPT_H
