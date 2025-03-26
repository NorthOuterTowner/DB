#ifndef LEXER_H
#define LEXER_H

#include <QObject>
#include <string>
#include <variant>
#include <vector>
#include <map>
#include <memory>
#include "dbManager.h"

// 前向声明
struct Condition;  // 进行前向声明
struct LogicalOp;  // 进行前向声明

// 定义 Node 类型别名
using Node = std::variant<Condition, LogicalOp>;

// 定义 SQLVal 类型别名
using SQLVal = std::variant<
    bool,
    std::string,
    std::vector<std::string>,
    std::vector<std::map<std::string, std::string>>,
    std::vector<std::vector<std::string>>,
    std::shared_ptr<Node> // 使用 Node (指针)
    >;

class Lexer : public QObject
{
    Q_OBJECT

public:
    Lexer();

public slots:
    void handleRawSQL(QString rawSql); // 这是一个槽函数

public:
    std::shared_ptr<Node> parseCondition(const std::string& conditionStr);
    std::shared_ptr<Node> parseLogicalOp(const std::string& logicalStr);
    std::shared_ptr<Node> parseWhereClause(const std::string& whereStr);
    std::map<std::string, SQLVal> parseCreate(const std::string& sql);
    std::map<std::string, SQLVal> parseDrop(const std::string& sql);
    std::map<std::string, SQLVal> parseInsert(const std::string& sql);
    std::map<std::string, SQLVal> parseUse(const std::string& sql);
    std::map<std::string, SQLVal> parseSelect(const std::string& sql);
    std::map<std::string, SQLVal> parseGrant(const std::string& sql);
    std::map<std::string, SQLVal> parseRevoke(const std::string& sql);
    std::map<std::string, SQLVal> parseAlter(const std::string& sql);
    std::map<std::string, SQLVal> parseShow(const std::string& sql);
    std::map<std::string, SQLVal> parseUpdate(const std::string& sql);
    std::map<std::string, SQLVal> parseDelete(const std::string& sql);
    std::map<std::string, SQLVal> parseDescribe(const std::string& sql);
    std::map<std::string, SQLVal> parseSQL(const std::string& sql); // 声明的功能

private:
    dbManager dbMgr; // 数据库管理器
};

#endif // LEXER_H
