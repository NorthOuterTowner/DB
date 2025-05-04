#ifndef TABLE_MANAGE_H
#define TABLE_MANAGE_H

#include <string>
#include <vector>
#include <QTreeWidget>
#include <iomanip>
#include <sstream>
#include <QFile>
#include <QTextStream>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "fieldmanage.h"
#include "affair.h"

class tableManage {
public:
    // 构造函数
    tableManage();
    // 创建表的函数
    bool createTable(const std::string& tableName, const std::string& dbName);
    // 检查表名是否有效
    bool isValidTableName(const std::string& dbName, const std::string& tableName);
    // 新增修改表的方法声明
    // 修改后的 alterTable 函数声明，增加了操作类型参数和多个字段信息参数
    bool alterTable(const std::string& dbName, const std::string& tableName, const std::string& operation,
                    const std::vector<std::string>& fieldNames, const std::vector<int>& fieldOrders,
                    const std::vector<std::string>& fieldTypes, const std::vector<int>& fieldTypeParams,
                    const std::vector<std::string>& constraints);

    bool dropTable(const std::string& dbName, const std::string& tableName);// 新增删除表的方法声明

    struct TableInfo {
        std::string table_name; // 表名称
        std::string databaseName; // 外键，表关联的数据库名
        std::string creation_date; // 创建日期时间
        std::string last_modified_date; // 最后修改时间
        int field_count = 0; // 表格字段数，初始为 0
        int record_count = 0; // 表中记录总数，初始为 0
        std::string table_type = "BASE"; // 表类型
    };

    //获取表信息
    TableInfo getTableInfo(const std::string& dbName,const std::string& tableName);

    //void backupTable(const std::string& dbName, const std::string& tableName);


private:

    std::string databaseName;// 表关联的数据库名
    std::string tableDescFile; // 表描述文件路径
    // 字符串分割函数
    std::vector<std::string> split(const std::string& s, const std::string& delimiter);
    Affair affair; // 声明 Affair 类型的成员变量
};

#endif
