#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <iomanip>
#include <sstream>
#include <QFile>
#include <QTextStream>
#include <QTreeWidget>
#include <vector>
#include <string>
#include "affair.h"


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
    void setCurrentDatabase(const std::string& dbName); // 设置当前使用的数据库
    void setCurrentTable(const std::string& tableName); // 设置当前使用的表
    // 新增函数，用于添加表到数据库的表列表中
    void addTableToDatabase(const std::string& dbName, const std::string& tableName);
    void dropTableFromDatabase(const std::string& dbName, const std::string& tableName);
    std::vector<std::string> getDatabaseTables(const std::string& dbName);
    void loadTableDescriptions();//加载数据库下列表的函数
    std::vector<std::string> split(const std::string& s, const std::string& delimiter);
    void backupDatabaseMetadata();
    std::string getCurrentDatabase()const;

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
    std::string currentDatabase; // 记录当前使用的数据库名称
    std::map<std::string, std::vector<std::string>> databaseTables;// 存储每个数据库拥有的表名
    // 在 dbManager.h 中添加函数声明
    Affair affair; // 声明 Affair 类型的成员变量

};

#endif // DBMANAGER_H
