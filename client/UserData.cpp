#include "UserData.h"

std::fstream& operator<<( std::fstream& out, const UserData& u ) {
    out << u.password << "\n";
    out << u.name << "\n";
    out << u.email << "\n";
    out << u.phone << std::endl;

    return out;
}

std::ostream& operator<<( std::ostream& out, const UserData& u ) {
    out << "1. Password: " << u.password << "\n";
    out << "2. Name: "<< u.name << "\n";
    out << "3. Email: " << u.email << "\n";
    out << "4. Phone: " << u.phone << std::endl;

    return out;
}