
#ifndef ADMIN_H
#define ADMIN_H

#include <QObject>

class Admin
{
public:
    Admin();
    static bool search(std::string & user,std::string & db,std::string & right);
    static bool grant(std::string & user,std::string & db,std::string & rights);
    static bool revoke(std::string & user,std::string & db,std::string & rights);
};

#endif // ADMIN_H

