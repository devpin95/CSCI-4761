#include "Appt.h"
std::fstream& operator<<( std::fstream& out, const Appt& a ) {
    out << a.ID << "\n";
    out << a.begin << "\n";
    out << a.end << "\n";
    out << a.place << "\n";
    out << a.contents << std::endl;

    return out;
}