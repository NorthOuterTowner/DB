#ifndef USERMANAGE_H
#define USERMANAGE_H

#include <string>
#include <vector>

class Wrong; // 前向声明

class UserManage {
public:
    UserManage();
    static void createUser(std::string & username, std::string & password);
    static void dropUser(std::string username);
    static bool findUser(std::string username, std::string password);

private:
    static std::string encryptPassword(const std::string& password); // 新增私有方法
};

#endif // USERMANAGE_H
