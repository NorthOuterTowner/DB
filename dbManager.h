#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <set>
#include <string>
#include <iostream>

class dbManager {
public:
    bool createDatabase(const std::string& name);
    bool dropDatabase(const std::string& name);

private:
    std::set<std::string> databases; // 存储现有数据库的集合
};

#endif // DBMANAGER_H
