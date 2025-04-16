#ifndef FIELD_MANAGE_H
#define FIELD_MANAGE_H

#include <string>
#include <vector>
#include <QFile>
#include <QTextStream>
#include <iostream>

class fieldManage {
public:
    // 构造函数
    fieldManage();
    // 添加字段的函数
    bool addField(const std::string& dbName, const std::string& tableName, const std::string& fieldName, int fieldOrder, const std::string& fieldType, int fieldTypeParams, const std::string& modificationTime, const std::string& constraints = "");
    // 删除字段的函数
    bool dropField(const std::string& dbName, const std::string& tableName, const std::string& fieldName);
private:
    // 定义 FieldInfo 结构体，用于存储字段的详细信息
    struct FieldInfo {
        std::string dbName;         // 所属数据库名称
        std::string tableName;      // 所在表名称
        std::string fieldName;      // 字段名
        int fieldOrder;             // 字段顺序
        std::string fieldType;      // 字段类型
        int fieldTypeParams;        // 字段类型参数
        std::string modificationTime; // 创建日期时间
        std::string constraints;    // 约束条件

        // 构造函数，方便初始化 FieldInfo
        FieldInfo(const std::string& db, const std::string& table, const std::string& field, int order,
                  const std::string& type, int params, const std::string& time,
                  const std::string& constraint = "")
            : dbName(db), tableName(table), fieldName(field), fieldOrder(order),
            fieldType(type), fieldTypeParams(params), modificationTime(time), constraints(constraint) {}
    };

    // 获取表描述文件地址
    std::string getTableDescFilePath(const std::string& dbName);
    // 获取表定义文件地址
    std::string getTableDefFilePath(const std::string& tableName);
    // 字段名是否合法
    bool isFieldNameValid(const std::string& fieldName);
    // 更新表描述文件
    bool updateTableDescFile(const FieldInfo& info, const std::string& operation);
    // 创建或更新表定义文件
    bool createOrUpdateTableDefFile(const FieldInfo& info, const std::string& operation);
    bool updateTableDescAndDefFiles(const FieldInfo& info, const std::string& operation);
    // 字符串分割函数
    std::vector<std::string> split(const std::string& s, const std::string& delimiter);
};

#endif // FIELD_MANAGE_H
