#include "dbManager.h"
#include <QDebug>

bool dbManager::createDatabase(const std::string& name) {
    if (databases.count(name) > 0) { // 检查数据库是否已存在
        std::cerr << "Database " << name << " already exists." << std::endl;
        return false; // 返回 false 表示创建失败
    }
    databases.insert(name); // 插入新数据库
    std::cout << "Database " << name << " created successfully." << std::endl;

    // 遍历集合并打印每个元素
    for (const auto& db : databases) {
        qDebug() << QString::fromStdString(db); // 将 std::string 转换为 QString
    }

    return true; // 返回 true 表示创建成功
}

bool dbManager::dropDatabase(const std::string& name) {
    if (databases.count(name) == 0) { // 检查数据库是否存在
        std::cerr << "Database " << name << " does not exist." << std::endl;
        return false; // 返回 false 表示删除失败
    }
    databases.erase(name); // 删除数据库
    std::cout << "Database " << name << " dropped successfully." << std::endl;

    // 遍历集合并打印每个元素
    for (const auto& db : databases) {
        qDebug() << QString::fromStdString(db); // 将 std::string 转换为 QString
    }

    return true; // 返回 true 表示删除成功
}
