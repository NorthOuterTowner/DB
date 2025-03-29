#ifndef USERMANAGE_H
#define USERMANAGE_H

#include <QObject>

class UserManage
{
public:
    UserManage();
    static void createUser(std::string,std::string);
    static void dropUser(std::string);
    static bool findUser(std::string,std::string);
};

#endif // USERMANAGE_H
