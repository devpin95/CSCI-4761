#include "UserData.h"

std::fstream& operator<<( std::fstream& out, const UserData& u ) {
    out << u.password << "\n";
    out << u.name << "\n";
    out << u.email << "\n";
    out << u.phone << std::endl;

    return out;
}