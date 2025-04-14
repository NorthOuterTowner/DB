#include "lexer.h"
#include "utils.h"
#include "dbManager.h"
#include "usermanage.h"
#include "databaselistdialog.h"
#include "admin.h"
#include <iostream>
#include <regex>
#include <map>
#include <vector>
#include <variant>
#include <memory>
#include <utility>
#include <QDebug>
#include <server.h>

Lexer::Lexer(QWidget *parent) : parentWidget(parent) {}


// 实现设置 QTreeWidget 指针的方法，传递给 dbManager
void Lexer::setTreeWidget(QTreeWidget* treeWidget) {
    this->treeWidget = treeWidget;
    dbMgr.setTreeWidget(treeWidget);
}

// 实现 reloadDbManagerDatabases 方法
void Lexer::reloadDbManagerDatabases() {
    dbMgr.reloadDatabases();
}

/*实现SHOW DATABASES语句的实现
void Lexer::setTextEdit(QTextEdit* textEdit) {
    this->textEdit = textEdit;
}*/

#define ICASE std::regex_constants::icase

using SQLVal = std::variant< bool, std::string, std::vector<std::string>,
                            std::vector<std::map<std::string,std::string>>,
                            std::vector<std::vector<std::string>>,std::shared_ptr<Node> >;

std::shared_ptr<Node> parseWhereClause(const std::string& whereStr);

// 解析单个条件
std::shared_ptr<Node> Lexer::parseCondition(const std::string& conditionStr) {
    std::regex conditionPattern(R"((\w+)\s*([=<>!]+)\s*('[^']*'|\w+))");
    std::smatch match;
    if (std::regex_search(conditionStr, match, conditionPattern)) {
        Condition condition;
        condition.column = match[1].str();
        condition.op = match[2].str();
        condition.value = match[3].str();
        return std::make_shared<Node>(condition);
    }
    return nullptr;
}

// 解析逻辑运算符
std::shared_ptr<Node> Lexer::parseLogicalOp(const std::string& logicalStr) {
    std::regex logicalPattern(R"((.*?)\s+(AND|OR)\s+(.*))", std::regex_constants::icase);
    std::smatch match;
    if (std::regex_search(logicalStr, match, logicalPattern)) {
        std::string leftStr = match[1].str();
        std::string op = match[2].str();
        std::string rightStr = match[3].str();

        LogicalOp logicalOp;
        logicalOp.op = op;
        logicalOp.left = parseWhereClause(leftStr);
        logicalOp.right = parseWhereClause(rightStr);

        return std::make_shared<Node>(logicalOp);
    }
    return nullptr;
}

// 解析WHERE子句
std::shared_ptr<Node> Lexer::parseWhereClause(const std::string& whereStr) {
    // 尝试解析逻辑运算符
    auto logicalNode = parseLogicalOp(whereStr);
    if (logicalNode) {
        return logicalNode;
    }

    // 如果没有逻辑运算符，则解析为单个条件
    auto conditionNode = parseCondition(whereStr);
    if (conditionNode) {
        return conditionNode;
    }

    return nullptr;
}

//where嵌套时括号优先
std::vector<std::string> Lexer::tokenize(const std::string& str) {
    std::vector<std::string> tokens;
    std::regex token_pattern(R"(\s*([()])\s*|\s*(AND|OR|=|<>|<|>|<=|>=)\s*|\s*([\w\.']+)\s*)", std::regex::icase);
    auto begin = std::sregex_iterator(str.begin(), str.end(), token_pattern);
    auto end = std::sregex_iterator();

    for (auto it = begin; it != end; ++it) {
        for (int i = 1; i < it->size(); ++i) {
            if ((*it)[i].matched) {
                std::string token = (*it)[i].str();
                std::transform(token.begin(), token.end(), token.begin(), ::toupper);
                tokens.push_back(token);
                break;
            }
        }
    }
    return tokens;
}

std::shared_ptr<Node> Lexer::parsWhereClause(const std::string& whereClause) {
    std::vector<std::string> tokens = tokenize(whereClause);
    int pos = 0;
    return parsExpression(tokens, pos);
}


std::shared_ptr<Node> Lexer::parsLogicalOp(const std::string& whereClause) {
    std::vector<std::string> tokens = tokenize(whereClause);
    int pos = 0;
    return parsExpression(tokens, pos);
}


// 处理 OR
std::shared_ptr<Node> Lexer::parsExpression(std::vector<std::string>& tokens, int& pos) {
    auto node = parseTerm(tokens, pos);
    while (pos < tokens.size() && tokens[pos] == "OR") {
        std::string op = tokens[pos++];
        auto right = parseTerm(tokens, pos);
        node = std::make_shared<Node>(LogicalOp{op, node, right});
    }
    return node;
}

// 添加这两个函数声明
//std::shared_ptr<Node> Lexer:: parseTerm(std::vector<std::string>& tokens, int& pos);
//std::shared_ptr<Node> parseFactor(std::vector<std::string>& tokens, int& pos);


// 处理 AND
std::shared_ptr<Node> Lexer::parseTerm(std::vector<std::string>& tokens, int& pos) {
    auto node = parseFactor(tokens, pos);
    while (pos < tokens.size() && tokens[pos] == "AND") {
        std::string op = tokens[pos++];
        auto right = parseFactor(tokens, pos);
        node = std::make_shared<Node>(LogicalOp{op, node, right});
    }
    return node;
}

// 处理括号或基本条件
std::shared_ptr<Node> Lexer::parseFactor(std::vector<std::string>& tokens, int& pos) {
    if (tokens[pos] == "(") {
        ++pos; // 跳过 (
        auto node = this->parsExpression(tokens, pos);
        if (tokens[pos] != ")") {
            throw std::runtime_error("Missing closing parenthesis");
        }
        ++pos; // 跳过 )
        return node;
    } else {
        // 是一个基本条件：col op val
        if (pos + 2 >= tokens.size()) {
            throw std::runtime_error("Invalid condition in WHERE clause");
        }
        Condition cond{tokens[pos], tokens[pos + 1], tokens[pos + 2]};
        pos += 3;
        return std::make_shared<Node>(cond);
    }
}

// 实现解析创建数据库的函数
std::map<std::string, SQLVal> Lexer::parseCreate(const std::string& sql) {

    std::map<std::string, SQLVal> result = { {"type", "CREATE"}, {"status", false} };
    std::regex createPattern(R"(CREATE\s+(DATABASE|TABLE|USER)\s+([\w@]+)\s*(?:\(([\s\S]*?)\))?\s*)", std::regex_constants::icase);
    std::smatch match;

    if (std::regex_search(sql, match, createPattern)) {
        result["status"] = true; // 状态为成功
        result["object_type"] = std::string(match[1].str());
        result["object_name"] = std::string(match[2].str());
        std::vector<std::string> userInfo=utils::split(std::string(std::get<std::string>(result["object_name"])),"@");
        if(std::get<std::string>(result["object_type"])=="USER"){
            std::cout<<"TRUE"<<std::endl;
            std::string name = userInfo[0];
            std::string pwd = userInfo[1];
            UserManage::createUser(name,pwd);
        }
        // 如果创建的是数据库
        if (std::get<std::string>(result["object_type"]) == "DATABASE") {
            // 使用 dbManager 创建数据库
            if (std::holds_alternative<std::string>(result["object_name"])) { // 检查类型

                std::string objectName = std::get<std::string>(result["object_name"]); // 提取

                if (dbMgr.createDatabase(objectName)) {
                    result["status"] = true; // 返回状态成功
                    std::cout << "Database created successfully." << std::endl;
                } else {
                    result["status"] = false; // 返回状态失败
                    std::cout << "Database created failed." << std::endl;
                }
            }
        } else if (std::get<std::string>(result["object_type"]) == "TABLE" && match[3].matched) {
            // 创建表的逻辑
            std::string columnsStr = match[3]; // 获取列定义字符串
            std::regex columnPattern(R"(\s*(\w+)\s+(\w+(?:\s*\([^)]*\))?)\s*(.*?)(?:,|$))");
            std::smatch columnMatch;
            std::vector<std::map<std::string, std::string>> columns;

            auto columnBegin = std::sregex_iterator(columnsStr.begin(), columnsStr.end(), columnPattern);
            auto columnEnd = std::sregex_iterator();

            for (std::sregex_iterator it = columnBegin; it != columnEnd; ++it) {
                std::map<std::string, std::string> column;
                column["name"] = (*it)[1].str();
                column["type"] = (*it)[2].str();
                std::string constraints = (*it)[3].str();

                //处理外键约束
                std::regex foreignKeyPattern(R"(FOREIGN KEY\s*\((\w+)\)\s*REFERENCES\s+(\w+)\s*\((\w+)\))");
                std::smatch foreignKeyMatch;
                if(std::regex_search(constraints,foreignKeyMatch,foreignKeyPattern)){
                    column["foreign_key_column"] = foreignKeyMatch[1].str();
                    column["referenced_table"] = foreignKeyMatch[2].str();
                    column["referenced_column"]= foreignKeyMatch[3].str();
                }

                column["constraints"]=constraints;
                columns.push_back(column);
            }
            result["columns"] = columns; // 将列信息存储到结果中
        }
    }
    return result;
}

// 实现解析删除数据库的函数
std::map<std::string, SQLVal> Lexer::parseDrop(const std::string& sql) {
    std::map<std::string, SQLVal> result = { {"type", std::string("DROP")}, {"status", false}, {"restrict", true} };
    std::regex pattern(R"(DROP\s+(DATABASE|TABLE|USER)\s+(\w+)\s*(RESTRICT|CASCADE)*)", std::regex_constants::icase);
    std::smatch match;

    if (std::regex_search(sql, match, pattern)) {
        result["status"] = true; // 状态为成功
        result["object_type"] = std::string(match[1].str());
        result["object_name"] = std::string(match[2].str());

        if(std::get<std::string>(result["object_type"])=="USER"){
            UserManage::dropUser(std::get<std::string>(result["object_name"]));
        }
        // 使用 dbManager 删除数据库
        if (std::get<std::string>(result["object_type"]) == "DATABASE") {
            if (std::holds_alternative<std::string>(result["object_name"])) { // 检查类型
                std::string objectName = std::get<std::string>(result["object_name"]); // 提取
                if (dbMgr.dropDatabase(objectName)) {
                    result["status"] = true; // 删除成功
                } else {
                    result["status"] = false; // 删除失败
                }
            }
        }

        // 处理 RESTRICT 或 CASCADE
        if (match[3].matched) {
            result["restrict"] = (match[3].str() == "RESTRICT");
        }
    }
    return result;
}

std::map<std::string, SQLVal> Lexer::parseInsert(const std::string& sql) {
    std::map<std::string, SQLVal> result = { {"type", "INSERT"}, {"status", false} };

    std::regex pattern(R"(^INSERT\s+INTO\s+(\w+)\s*\(([^)]+)\)\s*VALUES\s*(\([\S\s]+\)))", ICASE);
    std::smatch match;

    if (std::regex_search(sql, match, pattern)) {
        result["status"] = true;
        result["table"] = std::string(match[1].str());

        std::vector<std::string> columns;
        std::regex colPattern(R"(\w+)");
        std::string columnsStr = match[2].str();
        auto colBegin = std::sregex_iterator(columnsStr.begin(), columnsStr.end(), colPattern);
        auto colEnd = std::sregex_iterator();

        for (auto it = colBegin; it != colEnd; ++it) {
            columns.push_back(it->str());
        }

        std::vector<std::vector<std::string>> valuesGroups;
        std::regex groupPattern(R"(\(([^()]+?)\))");
        std::string valStr = match[3].str();
        auto groupBegin = std::sregex_iterator(valStr.begin(), valStr.end(), groupPattern);
        auto groupEnd = std::sregex_iterator();

        for (auto it = groupBegin; it != groupEnd; ++it) {
            std::string groupValues = it->str(1);
            std::vector<std::string> values;

            std::regex valPattern(R"((?:'[^']*'|[^,]+))");
            auto valBegin = std::sregex_iterator(groupValues.begin(), groupValues.end(), valPattern);
            auto valEnd = std::sregex_iterator();

            for (auto valIt = valBegin; valIt != valEnd; ++valIt) {
                std::string val = valIt->str();
                val.erase(0, val.find_first_not_of(' '));
                val.erase(val.find_last_not_of(' ') + 1);
                values.push_back(val);
            }
            if (columns.size() == values.size()) {
                valuesGroups.push_back(values);
            }
        }

        result["values"] = valuesGroups;
    }

    return result;
}

std::map<std::string, SQLVal> Lexer::parseUse(const std::string& sql){
    std::map<std::string, SQLVal> result = { {"type", std::string("USE")}, {"status", false} };
    std::regex pattern(R"(USE\s+(\w+))",ICASE);
    std::smatch match;

    if(std::regex_search(sql,match,pattern)){
        result["status"] = true;
        std::string dbName = std::string(match[1].str());
        result["name"] = dbName;
        currentDatabase = dbName; // 更新当前使用的数据库名称
        dbMgr.setCurrentDatabase(dbName); // 通知 dbManager 当前使用的数据库
    }
    return result;
}

/**Test Finished 
 * Wait for examination
 * TODO:Need to parse "AS" and conditions
*/
std::map<std::string, SQLVal> Lexer::parseSelect(const std::string& sql) {
    std::map<std::string, SQLVal> result = { {"type", std::string("SELECT")}, {"status", false} };
    std::regex pattern(R"(SELECT\s+(.*?)\s+FROM\s+(\w+)(?:\s+WHERE\s+(.*?))?(?:\s+GROUP\s+BY\s+(.*?))?(?:\s+HAVING\s+(.*?))?(?:\s+ORDER\s+BY\s+(.*?))?(?:\s+LIMIT\s+(.*?))?)", ICASE);
    std::smatch match;

    if (std::regex_search(sql, match, pattern)) {
        result["status"] = true;

        std::regex colRegex(R"(\w+(?:\s+AS\s+\w+)?)", ICASE);
        std::string matchStr = match[1].str();
        auto colBegin = std::sregex_iterator(matchStr.begin(), matchStr.end(), colRegex);
        auto colEnd = std::sregex_iterator();
        std::vector<std::string> columns;
        for (auto it = colBegin; it != colEnd; ++it) {
            columns.push_back(it->str());
        }
        result["columns"] = columns;

        result["table"] = std::string(match[2].str());
        if (match[3].matched) result["where"] = parseWhereClause(std::string(match[3].str()));
        if (match[4].matched) result["group_by"] = std::string(match[4].str());
        if (match[5].matched) result["having"] = std::string(match[5].str());
        if (match[6].matched) result["order_by"] = std::string(match[6].str());
        if (match[7].matched) result["limit"] = std::string(match[7].str());
    }
    return result;
}

std::map<std::string, SQLVal> Lexer::parseGrant(const std::string& sql) {
    std::map<std::string, SQLVal> result = { {"type", std::string("GRANT")}, {"status", false} };
    std::regex pattern(R"(GRANT\s+([\w,\s]+)\s+ON\s+(\w+)\s+TO\s+(\w+))", ICASE);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = true;
        result["object"] = std::string(match[2].str());//db&table
        result["user"] = std::string(match[3].str());//user
        std::vector<std::string> rightsList;
        std::regex rightsRegex(R"(\w+)");
        std::string matchStr = match[1].str();
        auto rightsBegin = std::sregex_iterator(matchStr.begin(), matchStr.end(), rightsRegex);
        auto rightsEnd = std::sregex_iterator();
        for (auto it = rightsBegin; it != rightsEnd; ++it) {
            std::string rightStr(it->str());
            rightsList.push_back(it->str());
            Admin::grant(std::get<std::string>(result["user"]),std::get<std::string>(result["object"]),rightStr);
        }

        result["rights"] = rightsList;
    }
    return result;
}

std::map<std::string, SQLVal> Lexer::parseRevoke(const std::string& sql) {
    std::map<std::string, SQLVal> result = { {"type", std::string("REVOKE")}, {"status", false} };
    std::regex pattern(R"(REVOKE\s+([\w,\s]+)\s+ON\s+(\w+)\s+FROM\s+(\w+))", ICASE);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = true;
        result["rights"] = std::string(match[1].str());
        result["object"] = std::string(match[2].str());
        result["user"] = std::string(match[3].str());

        std::vector<std::string> rightsList;
        std::regex rightsRegex(R"(\w+)");
        std::string matchStr = match[1].str();
        auto rightsBegin = std::sregex_iterator(matchStr.begin(), matchStr.end(), rightsRegex);
        auto rightsEnd = std::sregex_iterator();
        for (auto it = rightsBegin; it != rightsEnd; ++it) {
            rightsList.push_back(it->str());
        }
        result["rights"] = rightsList;
    }
    return result;
}

/**Test Finished 
 * Wait for examination
*/
std::map<std::string, SQLVal> Lexer::parseAlter(const std::string& sql) {
    std::map<std::string, SQLVal> result = { {"type", std::string("ALTER")}, {"status", false} } ;
    std::regex pattern(R"(ALTER\s+TABLE\s+(\w+)\s+(ADD|MODIFY|DROP)\s+([\s\S]*);$)", ICASE);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = true;
        result["table"] = std::string(match[1].str());
        result["action"] = std::string(match[2].str());
        std::string columnsStr = std::string(match[3].str());

        std::vector<std::map<std::string, std::string>> columns;
        std::regex columnPattern(R"((\w+)\s+([\w\(\)]+)\s*([^,]*))");
        auto columnBegin = std::sregex_iterator(columnsStr.begin(), columnsStr.end(), columnPattern);
        auto columnEnd = std::sregex_iterator();
        for (std::sregex_iterator it = columnBegin; it != columnEnd; ++it) {
            std::map<std::string, std::string> column;
            column["name"] = utils::strip((*it)[1].str());
            column["type"] = utils::strip((*it)[2].str());
            column["constraints"] = utils::strip((*it)[3].str());
            columns.push_back(column);
        }
        result["columns"] = columns;
    }
    return result;
}

std::map<std::string, SQLVal> Lexer::parseShow(const std::string& sql) {
    std::map<std::string, SQLVal> result = { {"type", std::string("SHOW")}, {"status", false} };
    std::regex pattern(R"(SHOW\s+(TABLES|DATABASES)(?:\s+FROM\s+(\w+))?)", std::regex::icase);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = true;
        result["item"] = std::string(match[1].str());
        if (match[2].matched) result["database"] = match[2];

        if (std::string(match[1].str()) == "DATABASES") {
            std::vector<std::string> dbNames = dbMgr.getDatabaseNames();
            QStringList qDbNames;
            for (const auto& name : dbNames) {
                qDbNames.append(QString::fromStdString(name));
            }

            DatabaseListDialog dialog(qDbNames, parentWidget);
            dialog.exec();
        }
    }
    return result;
}


std::map<std::string, SQLVal> Lexer::parseUpdate(const std::string& sql) {
    std::map<std::string, SQLVal> result = { {"type", std::string("UPDATE")}, {"status", false} };
    std::regex pattern(R"(UPDATE\s+(\w+)\s+SET\s+(\w+)\s*=\s*(\w+)\s+WHERE\s+(.+);$)", ICASE);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = true;
        result["table"] = std::string(match[1].str());
        result["column"] = std::string(match[2].str());
        result["value"] = std::string(match[3].str());
        if (match[4].matched) result["condition"] = std::string(match[4].str());
    }
    return result;
}

/**Test Finished 
 * Wait for examination
 * TODO:Need an extra function to parse the condition
*/
std::map<std::string, SQLVal> Lexer::parseDelete(const std::string& sql) {
    std::map<std::string, SQLVal> result = { {"type", std::string("DELETE")}, {"status", false} };
    std::regex pattern(R"(DELETE\s+FROM\s+(\w+)(?:\s+WHERE\s+(.+))?;$)", ICASE);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = true;
        result["table"] = std::string(match[1].str());
        if (match[2].matched) result["condition"] = match[2];
    }
    return result;
}
/**Test Finished 
 * Wait for examination
*/
std::map<std::string, SQLVal> Lexer::parseDescribe(const std::string& sql) {
    std::map<std::string, SQLVal> result = { {"type", std::string("DESCRIBE")}, {"status", false} };
    std::regex pattern(R"(DESCRIBE\s+(\w+)(?:\s+(\w+))?;$)", ICASE);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = true;
        result["table"] = std::string(match[1]);
        if (match[2].matched) result["column"] = match[2];
    }
    return result;
}

using ParseFunc = std::map<std::string, SQLVal>(Lexer::*)(const std::string&);

std::map<std::string, SQLVal> Lexer::parseSQL(const std::string& sql) {
    std::vector<std::pair<std::regex, ParseFunc>> patterns = {
        {std::regex(R"(^CREATE\s)", ICASE), &Lexer::parseCreate},
        {std::regex(R"(^INSERT\s)", ICASE), &Lexer::parseInsert},
        {std::regex(R"(^SELECT\s)", ICASE), &Lexer::parseSelect},
        {std::regex(R"(^USE\s)", ICASE), &Lexer::parseUse},
        {std::regex(R"(^DROP\s)", ICASE), &Lexer::parseDrop},
        {std::regex(R"(^GRANT\s)", ICASE), &Lexer::parseGrant},
        {std::regex(R"(^REVOKE\s)", ICASE), &Lexer::parseRevoke},
        {std::regex(R"(^ALTER\s)", ICASE), &Lexer::parseAlter},
        {std::regex(R"(^SHOW\s)", ICASE), &Lexer::parseShow},
        {std::regex(R"(^UPDATE\s)", ICASE), &Lexer::parseUpdate},
        {std::regex(R"(^DELETE\s)", ICASE), &Lexer::parseDelete},
        {std::regex(R"(^DESCRIBE\s)", ICASE), &Lexer::parseDescribe}
    };

    for (const auto& [pattern, func] : patterns) {
        if (std::regex_search(sql, pattern)) {
            return (this->*func)(sql);
        }
    }
    return { {"type", std::string("UNKNOWN")}, {"status", false} };
}

void Lexer::handleRawSQL(QString rawSql){
    std::vector<std::string> sqls=utils::split(rawSql.toStdString(),";");
    for(std::string sql:sqls){
        std::map<std::string,SQLVal> result = parseSQL(sql);
        std::string type = std::get<std::string>(result["type"]);
        bool status = std::get<bool>(result["status"]);
        std::cout << "Type: " << type << ", Status: " << status << std::endl;
        if(status){
            Server * server = Server::getInstance();
            server->sendMessageToServer(QString::fromStdString(sql));
        }
    }
}

