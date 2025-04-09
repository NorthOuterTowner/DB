#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <iomanip>
#include <sstream>
#include <QFile>
#include <QTextStream>
#include <QTreeWidget>
#include <vector>
#include <string>

class dbManager {
public:
    dbManager();

    // 设置 QTreeWidget 指针的方法
    void setTreeWidget(QTreeWidget* treeWidget);

    bool createDatabase(const std::string& name);
    bool dropDatabase(const std::string& name);
    void loadDatabases(); // 从文件加载现有数据库
    void saveDatabases(); // 保存数据库到文件
    std::vector<std::string> getDatabaseNames(); // 用于SHOW DATABASES
    void reloadDatabases();//用于初始化db_list
    void setCurrentDatabase(const std::string& dbName); // 新增：设置当前使用的数据库

private:
    struct DatabaseInfo {
        int database_id; // 数据库ID
        std::string database_name; // 数据库名称
        std::string creation_date; // 创建日期时间
        std::string character_set; // 字符集
        std::string collation; // 排序规则
    };

    std::vector<DatabaseInfo> databases; // 存储数据库对象
    const std::string dbFilePath = "../../res/databases.txt"; // 数据库文件路径
    QTreeWidget* db_list; // 指向 QTreeWidget 的指针
    std::string currentDatabase; // 新增：记录当前使用的数据库名称
};

#endif // DBMANAGER_H
