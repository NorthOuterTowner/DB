#ifndef LEXER_H
#define LEXER_H

#include <QObject>
#include <string>
#include <variant>
#include <vector>
#include <map>
#include <memory>
#include <QTextEdit>
#include <QWidget>
#include "dbManager.h"
#include "databaselistdialog.h"

struct Node; // 前向声明

// 然后定义具体结构体
struct Condition {
    std::string column;
    std::string op;
    std::string value;
};

struct LogicalOp {
    std::string op; // "AND" 或 "OR"
    std::shared_ptr<Node> left;  // 使用指针避免立即需要完整定义
    std::shared_ptr<Node> right; // 使用指针避免立即需要完整定义
};

// 最后定义Node类型
struct Node : std::variant<Condition, LogicalOp> {
    using variant::variant; // 继承构造函数
};

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
    Lexer(QWidget *parent = nullptr);

public slots:
    void handleRawSQL(QString rawSql); // 这是一个槽函数

public:
    // 新增设置 QTreeWidget 指针的方法
    void setTreeWidget(QTreeWidget* treeWidget);//用于在db_list中添加或删除数据库
    void reloadDbManagerDatabases();//用于初始化db_list
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
    std::map<std::string, SQLVal> parseSQL(const std::string& sql);
    //void setTextEdit(QTextEdit* textEdit); // 用于SHOW DATABASES命令

private:
    dbManager dbMgr; // 数据库管理器
    QTreeWidget* treeWidget;// 用于在目录下显示数据库
    //QTextEdit* textEdit; // 用于SHOW DATABASES命令
    QWidget *parentWidget; // 新增成员变量，用于保存父窗口指针
    std::string currentDatabase; // 新增：记录当前使用的数据库名称
};

#endif // LEXER_H
