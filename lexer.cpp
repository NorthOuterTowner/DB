#include "lexer.h"
#include "utils.h"
#include "dbManager.h"
#include "usermanage.h"
#include "databaselistdialog.h"
#include "admin.h"

#include "logger.h"
#include "session.h"

#include "wrong.h"
#include "fieldmanage.h"

#include <iostream>
#include <regex>
#include <map>
#include <vector>
#include <variant>
#include <memory>
#include <utility>
#include <QDebug>
#include <server.h>

Lexer::Lexer(QWidget *parent) : parentWidget(parent),affair("undo.txt",this) {
    dataMgr=new datamanager(&dbMgr);

}
Lexer::~Lexer(){delete dataMgr;}
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
}
}*/

#define ICASE std::regex_constants::icase

using SQLVal = std::variant< bool, std::string, std::vector<std::string>,
                            std::vector<std::map<std::string,std::string>>,
                            std::vector<std::vector<std::string>>,std::shared_ptr<Node>,std::vector<SortRule> ,std::map<std::string,std::string>>;

//std::shared_ptr<Node> parseWhereClause(const std::string& whereStr);

// 解析单个条件
std::shared_ptr<Node> Lexer::parseCondition(const std::string& conditionStr) {
    std::regex conditionPattern(R"((\w+)\s*([=<>!]+)\s*('[^']*'|\w+))", std::regex::icase);
    std::smatch match;

    if (std::regex_search(conditionStr, match, conditionPattern)) {
        Condition condition;
        condition.column = match[1].str();  // 获取列名
        condition.op = match[2].str();      // 获取运算符
        condition.value = match[3].str();   // 获取值

        // 打印调试信息
        std::cout << "Parsed condition: "
                  << "column: " << condition.column
                  << ", operator: " << condition.op
                  << ", value: " << condition.value << std::endl;

        return std::make_shared<Node>(condition);
    }

    // 如果没有匹配条件，打印并返回 nullptr
    std::cout << "Invalid condition: " << conditionStr << std::endl;
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

    // 修复后的正则表达式：处理括号、操作符、字符串、数字和标识符
    std::regex token_pattern(
        R"((\()|(\))|(AND|OR|<>|<=|>=|=|<|>)|('[^']*')|(\d+)|([a-zA-Z_][a-zA-Z0-9_]*))",
        std::regex::icase
        );

    auto begin = std::sregex_iterator(str.begin(), str.end(), token_pattern);
    auto end = std::sregex_iterator();

    for (auto it = begin; it != end; ++it) {
        std::string token = it->str();
        // 逻辑关键字统一为大写
        if (token == "and" || token == "AND") token = "AND";
        else if (token == "or" || token == "OR") token = "OR";
        tokens.push_back(token);
    }

    return tokens;
}

std::shared_ptr<Node> Lexer::parsWhereClause(const std::string& whereClause) {
    std::string cleaned = whereClause;
    std::cout << "[DEBUG] Raw WHERE clause: " << whereClause << std::endl;
    if (!cleaned.empty() && cleaned.back() == ';') {
        cleaned.pop_back();  // 去掉结尾分号
    }

    std::vector<std::string> tokens = tokenize(cleaned);
    std::cout << "Tokens parsed from WHERE clause:\n";
    for (const auto& token : tokens) {
        std::cout << "[" << token << "] ";
    }
    std::cout << std::endl;
    if (tokens.empty()) {
        throw std::runtime_error("Invalid WHERE clause: no tokens found");
        }

    int pos = 0;
    auto node = parsExpression(tokens, pos);

    // 防止遗漏括号等错误
    if (pos < tokens.size()) {
        std::stringstream error_msg;
        error_msg << "Unexpected tokens after valid expression: ";
        for (size_t i = pos; i < tokens.size(); ++i) {
            error_msg << tokens[i] << " ";
        }
        throw std::runtime_error(error_msg.str());
    }

    //std::vector<std::string> tokens = tokenize(cleaned);


    return node;
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
    if (pos >= tokens.size()) {
        throw std::runtime_error("Unexpected end of tokens in WHERE clause");
    }

    if (tokens[pos] == "(") {
        ++pos; // 跳过 (
        auto node = this->parsExpression(tokens, pos);
        if (pos >= tokens.size() || tokens[pos] != ")") {
            throw std::runtime_error("Missing closing parenthesis");
        }
        ++pos; // 跳过 )
        return node;
    } else {
        // 是一个基本条件：col op val
        std::cout << "tokens size = " << tokens.size() << ", pos = " << pos << std::endl;
        if (pos + 2 >= tokens.size()) {
            throw std::runtime_error("Invalid condition in WHERE clause (not enough tokens)");
        }

        std::string col = tokens[pos++];
        std::string op = tokens[pos++];
        std::string val = tokens[pos++];

        for (const auto& t : tokens) {
            std::cout << "[" << t << "] ";
        }
        std::cout << std::endl;

        Condition cond{col, op, val};
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

                    // 记录日志
                    //Logger logger("../../res/system_logs.txt");
                    //logger.log(Session::getCurrentUserId(), "CREATE", "DATABASE", objectName); // 记录日志

                    std::cout << "Database created successfully." << std::endl;
                } else {
                    result["status"] = false; // 返回状态失败
                    std::cout << "Database created failed." << std::endl;
                }
            }
        } else if (std::get<std::string>(result["object_type"]) == "TABLE" ) {
            // 使用 tableManager 创建表
            std::string objectName = std::get<std::string>(result["object_name"]); // 提取

            if (tableMgr.createTable(objectName, currentDatabase)) {
                // 通知 dbManager 该数据库新增了一个表
                dbMgr.addTableToDatabase(currentDatabase, objectName);

                // 记录日志
                //Logger logger("../../res/system_logs.txt");
                //logger.log(Session::getCurrentUserId(), "CREATE", "TABLE", objectName + " in " + currentDatabase); // 记录日志


                result["status"] = true; // 返回状态成功
                // 调试
                std::cout << "Table created successfully." << std::endl;

            }
            else {
                result["status"] = false; // 返回状态失败
                // 调试
                std::cout << "Table created failed." << std::endl;
            }

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

        if (std::get<std::string>(result["object_type"]) == "USER") {
            UserManage::dropUser(std::get<std::string>(result["object_name"]));
        } else if (std::get<std::string>(result["object_type"]) == "DATABASE") {
            if (std::holds_alternative<std::string>(result["object_name"])) { // 检查类型
                std::string objectName = std::get<std::string>(result["object_name"]); // 提取
                if (dbMgr.dropDatabase(objectName)) {
                    result["status"] = true; // 删除成功

                    // 记录日志
                    //Logger logger("../../res/system_logs.txt");
                    //logger.log(Session::getCurrentUserId(), "DROP", "DATABASE", std::get<std::string>(result["object_name"])); // 记录日志
                } else {
                    result["status"] = false; // 删除失败
                }
            }
        } else if (std::get<std::string>(result["object_type"]) == "TABLE") {
            if (tableMgr.dropTable(currentDatabase, std::get<std::string>(result["object_name"]))) {
                // 调用 dbManager 的 dropTableFromDatabase 函数
                dbMgr.dropTableFromDatabase(currentDatabase, std::get<std::string>(result["object_name"]));

                // 记录日志
                //Logger logger("../../res/system_logs.txt");
                //logger.log(Session::getCurrentUserId(), "DROP", "TABLE", std::get<std::string>(result["object_name"]) + " from " + currentDatabase); // 记录日志

                result["status"] = true; // 删除成功
            } else {
                result["status"] = false; // 删除失败
            }
        }

        // 处理 RESTRICT 或 CASCADE
        if (match[3].matched) {
            result["restrict"] = (match[3].str() == "RESTRICT");
        }
    }
    return result;
}



std::vector<std::string> Lexer::getAllColumnsFromTable(const std::string& dbName, const std::string& tableName) {
    std::vector<std::string> columns;

    // 获取表信息
    tableManage::TableInfo tableInfo = tableMgr.getTableInfo(dbName, tableName);
    if (tableInfo.table_name.empty()) {
        std::cerr << "[error] 表 " << tableName << " 在数据库 " << dbName << " 中不存在。" << std::endl;
        return columns;
    }

    // 获取字段信息
    std::vector<fieldManage::FieldInfo> columnsInfo = fieldMgr.getFieldsInfo(dbName, tableName);
    for (const auto& field : columnsInfo) {
        columns.push_back(field.fieldName);
    }

    if (columns.empty()) {
        std::cerr << "[error] 未能获取表 " << tableName << " 的字段信息。" << std::endl;
    }

    return columns;
}





std::map<std::string, SQLVal> Lexer::parseInsert(const std::string& sql) {
    if(affair.isrunning){
        affair.writeToUndo(QString::fromStdString(sql));
    }
    std::map<std::string, SQLVal> result = { {"type", "INSERT"}, {"status", false} };
    
    std::string dbName=getCurrentDatabase();
    if(dbName.empty()){
        std::cerr << "[error]do not choose database ，can't use INSERT。" << std::endl;
        throw std::runtime_error("No database selected.");
    }

    // 匹配 INSERT INTO table (col1, col2, ...) VALUES (val1, val2), ...
    //std::regex pattern(R"(^INSERT\s+INTO\s+(\w+)\s*\(([^)]+)\)\s*VALUES\s*(\([\S\s]+\))\s*;?\s*$)", ICASE);
    std::regex pattern(R"(^INSERT\s+INTO\s+(\w+)(?:\s*\(([^)]+)\))?\s*VALUES\s*(\([\S\s]+\))\s*;?\s*$)", ICASE);
    std::smatch match;

    if (std::regex_search(sql, match, pattern)) {
        std::string tableName = match[1].str();
        std::string columnsStr = match[2].str();
        std::string valuesStr = match[3].str();

        std::cout << "[parse] insert table: " << tableName << std::endl;
        std::cout << "[parse] currentdatabase: " << dbName << std::endl;

        // 设置当前表名
        setCurrentTable(tableName);

        result["status"] = true;
        result["table"] = tableName;

        // // 解析列名
        // std::vector<std::string> columns;
        // std::regex colPattern(R"(\w+)");
        // auto colBegin = std::sregex_iterator(columnsStr.begin(), columnsStr.end(), colPattern);
        // auto colEnd = std::sregex_iterator();
        // for (auto it = colBegin; it != colEnd; ++it) {
        //     columns.push_back(it->str());
        // }

        std::vector<std::string> columns;
        if (columnsStr.empty()) {
            columns = getAllColumnsFromTable(dbName,tableName);  // Get all columns if not specified
        } else {
            std::regex colPattern(R"(\w+)");
            auto colBegin = std::sregex_iterator(columnsStr.begin(), columnsStr.end(), colPattern);
            auto colEnd = std::sregex_iterator();
            for (auto it = colBegin; it != colEnd; ++it) {
                columns.push_back(it->str());
            }
        }

        // 解析值组
        std::vector<std::vector<std::string>> valuesGroups;
        std::regex groupPattern(R"(\(([^()]+?)\))");
        auto groupBegin = std::sregex_iterator(valuesStr.begin(), valuesStr.end(), groupPattern);
        auto groupEnd = std::sregex_iterator();

        for (auto it = groupBegin; it != groupEnd; ++it) {
            std::string groupValues = it->str(1); // 括号内内容
            std::vector<std::string> values;

            // 支持字符串值：'Tom', 20
            std::regex valPattern(R"((?:'[^']*'|[^,]+))");
            auto valBegin = std::sregex_iterator(groupValues.begin(), groupValues.end(), valPattern);
            auto valEnd = std::sregex_iterator();

            for (auto valIt = valBegin; valIt != valEnd; ++valIt) {
                std::string val = valIt->str();
                // 去除前后空格和引号
                val.erase(0, val.find_first_not_of(" '"));
                val.erase(val.find_last_not_of(" '") + 1);
                values.push_back(val);
            }

            if (columns.size() == values.size()) {
                valuesGroups.push_back(values);
            } else {
                std::cerr << "[!] 列数与值数不匹配，忽略该组值" << std::endl;
            }
        }

        result["values"] = valuesGroups;

        // 解析成功后
        std::cout << "[success] all " << valuesGroups.size() << " 组插入值。" << std::endl;

        // 日志记录插入操作
        auto status = std::get_if<bool>(&result["status"]); // 提取状态值的指针
        if (status && *status) { // 只有在状态为 true 时记录日志
            Logger logger("../../res/system_logs.txt");
            logger.log(Session::getCurrentUserId(), "INSERT", "TABLE", sql); // 记录插入的 SQL 语句
        }

    return result;
}
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

        // 记录日志
        Logger logger("../../res/system_logs.txt"); // 指定日志文件路径
        logger.log(Session::getCurrentUserId(), "USE", "DATABASE", dbName); // 记录使用数据库的日志
    }
    return result;
}



// std::map<std::string, SQLVal> Lexer::parseSelect(const std::string& sql) {
//     std::map<std::string,SQLVal>result = {{"type","SELECT"},{"status","false"}};

//     std::string dbName=getCurrentDatabase();
//     if(dbName.empty()){
//         throw std::runtime_error("No database selected.");
//     }

//     std::regex pattern(
//         //R"(SELECT\s+(\*|[\w\s,]+)\s+FROM\s+(\w+)(?:\s+WHERE\s+([^\n;]+))?(?:\s+ORDER\s+BY\s+([^\n;]+))?)",
//         R"(SELECT\s+([\w\s,\(\)\*]+)\s+FROM\s+(\w+)(?:\s+WHERE\s+([^;]+))?(?:\s+GROUP\s+BY\s+([^;]+))?(?:\s+ORDER\s+BY\s+([^;]+))?)",

//         std::regex::icase                );
//     std::smatch match;

//     if (!std::regex_search(sql, match, pattern)) {
//         return result; // 无法解析
//     }

//     result["status"] = true;

//     // 1. 解析列名（支持 *）和聚合函数
//     std::string columnsStr = utils::strip(match[1].str());
//     std::vector<std::string> columns;
//     std::vector<std::string> aggregateFunctions;
//     if (columnsStr == "*") {
//         columns = {}; // 空数组代表 SELECT *
//     } else {
//         std::vector<std::string> rawColumns = split(columnsStr, ",");
//         // for (auto& col : rawColumns) {
//         //     col = utils::strip(col);
//         //     if (!col.empty()) {
//         //         columns.push_back(col);
//         //     }
//         for (auto& col : rawColumns) {
//             col = utils::strip(col);
//             if (col.find("(") != std::string::npos) {
//                 aggregateFunctions.push_back(col);
//             } else {
//                 columns.push_back(col);
//             }
//         }
//     }
//     result["columns"] = columns;
//     result["aggregates"] = aggregateFunctions;

//     // 2. 解析表名
//     result["table"] = utils::strip(match[2].str());

//     // 3. 解析 WHERE 子句（使用括号解析器）
//     if (match[3].matched) {
//         std::string whereStr = utils::strip(match[3].str());
//         std::cout << "[DEBUG] Raw WHERE clause: " << whereStr << std::endl;
//         result["whereTree"] = parsWhereClause(whereStr);
//     }

//     // 4. 解析 GROUP BY 子句
//     if (match[4].matched) {
//         std::vector<std::string> groupByColumns = split(match[4].str(), ",");
//         result["group_by"] = groupByColumns;
//     }



//     // 5. 解析 ORDER BY 子句
//     if (match[5].matched) {
//         std::vector<SortRule> sortRules;
//         //std::string orderByStr = match[4].str();
//         std::vector<std::string> orderTokens = split(match[5].str(), ",");
//         for (auto& token : orderTokens) {
//             token = utils::strip(token);
//             if (token.empty()) continue;

//             SortRule rule;
//             size_t spacePos = token.find_last_of(" \t"); // 查找排序关键字
//             if (spacePos != std::string::npos) {
//                 std::string col = token.substr(0, spacePos);
//                 std::string order = token.substr(spacePos + 1);
//                 rule.column = utils::strip(col);
//                 rule.isAscending = (order == "ASC" || order.empty());
//             } else {
//                 rule.column = token;
//                 rule.isAscending = true; // 默认升序
//             }
//             sortRules.push_back(rule);
//         }
//         result["order_by"] = sortRules;
//     }

//     std::cout << "[DEBUG] Full SQL: " << sql << std::endl;
//     if (match[3].matched) {
//         std::cout << "[DEBUG] WHERE match: " << match[3].str() << std::endl;
//     }

//     return result;
// }


//<<<<<<< HEAD




// Assume SQLVal can hold different types, e.g., using std::variant or specific keys
// struct SelectStatement { ... }; // Ideal structure as discussed in thought process

std::map<std::string, SQLVal> Lexer::parseSelect(const std::string& sql) {
    std::map<std::string, SQLVal> result = {{"type", "SELECT"}, {"status", false}};

// =======
// std::map<std::string, SQLVal> Lexer::parseSelect(const std::string& sql) {
//     std::map<std::string, SQLVal> result = { {"type", std::string("SELECT")}, {"status", false} };

// >>>>>>> 9e3794f9e4ed8e4a413a17e5976ebb9162f7bbed
    std::string dbName = getCurrentDatabase();
    if (dbName.empty()) {
        throw std::runtime_error("No database selected.");
    }

    std::regex pattern(
//<<<<<<< HEAD
        R"(SELECT\s+(?:DISTINCT\s+)?([\w\s,\(\)\*]+)\s+FROM\s+(\w+)(?:\s+WHERE\s+([^;]+))?(?:\s+GROUP\s+BY\s+([^;]+))?(?:\s+ORDER\s+BY\s+([^;]+))?)",
        std::regex::icase
        );
    std::smatch match;

    if (!std::regex_search(sql, match, pattern)) {
        std::cout << "[DEBUG] Failed to parse SQL: " << sql << std::endl;
        return result;
    }

    result["status"] = true;
    std::cout << "[DEBUG] Successfully parsed SQL: " << sql << std::endl;

    // 解析 SELECT 列表
    std::string selectListStr = utils::strip(match[1].str());
    std::vector<std::string> columns;
    std::vector<std::string> aggregateFunctions;

    if (selectListStr == "*") {
        //columns = {}; // SELECT * 表示所有列
        columns.push_back("*");//标记为特殊值而不是空数组
        std::cout << "[DEBUG] Detected SELECT * syntax" << std::endl;
    } else {
        std::vector<std::string> tokens = split(selectListStr, ",");
        for (auto& token : tokens) {
            token = utils::strip(token);
            if (token.empty()) continue;

            size_t openBracket = token.find('(');
            size_t closeBracket = token.find(')');

            if (openBracket != std::string::npos && closeBracket != std::string::npos && closeBracket > openBracket) {
                aggregateFunctions.push_back(token);
            } else {
                columns.push_back(token);
            }
        }
    }

    // 显式设置 columns 和 aggregates
    result["columns"] = columns;
    result["aggregates"] = aggregateFunctions;

    // 调试输出
    if (columns.size() == 1 && columns[0] == "*") {
        std::cout << "[DEBUG] Columns: 1 column (SELECT *)" << std::endl;
    } else {
        std::cout << "[DEBUG] Columns: " << columns.size() << " columns" << std::endl;
        for (const auto& col : columns) {
            std::cout << "[DEBUG]   Column: " << col << std::endl;
        }
    }

    std::cout << "[DEBUG] Aggregates: " << aggregateFunctions.size() << " functions" << std::endl;
    for (const auto& agg : aggregateFunctions) {
        std::cout << "[DEBUG]   Aggregate: " << agg << std::endl;
    }

    // 检查并设置结果
    if (!columns.empty()) {
        result["columns"] = columns;
    }
    if (!aggregateFunctions.empty()) {
        result["aggregates"] = aggregateFunctions;
    }

    // 解析表名
    std::string tableName = utils::strip(match[2].str());
    result["table"] = SQLVal(tableName);

    // 记录日志
    Logger logger("../../res/system_logs.txt");
    logger.log(Session::getCurrentUserId(), "SELECT", "TABLE", std::get<std::string>(result["table"]) + " from " + dbName); // 记录日志


    // 解析 WHERE 子句
    if (match[3].matched) {
        std::string whereStr = utils::strip(match[3].str());
        std::cout << "[DEBUG] WHERE clause found: " << whereStr << std::endl;

        try {
            std::shared_ptr<Node> whereTree = parsWhereClause(whereStr);
            result["whereTree"] = SQLVal(whereTree);
            std::cout << "[DEBUG] WHERE clause parsed successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Failed to parse WHERE clause: " << e.what() << std::endl;
            result["whereTree"] = SQLVal();
        }
    } else {
        result["whereTree"] = SQLVal();
        std::cout << "[DEBUG] No WHERE clause found" << std::endl;
    }

    // 解析 GROUP BY 子句
    std::vector<std::string> groupByColumns;
    if (match[4].matched) {
        std::string groupByStr = utils::strip(match[4].str());
        groupByColumns = split(groupByStr, ",");
        for (auto& col : groupByColumns) {
            col = utils::strip(col);
        }
        result["group_by"] = SQLVal(groupByColumns);
    } else {
        result["group_by"] = SQLVal(std::vector<std::string>());
    }

    // 解析 ORDER BY 子句
    std::vector<SortRule> sortRules;
    if (match[5].matched) {
        std::string orderByStr = utils::strip(match[5].str());
        std::vector<std::string> orderTokens = split(orderByStr, " ");

        for (auto& token : orderTokens) {
            token = utils::strip(token);
            if (token.empty()) continue;

            SortRule rule;
            size_t ascPos = utils::toUpper(token).find(" ASC");
            size_t descPos = utils::toUpper(token).find(" DESC");

            if (descPos != std::string::npos && (ascPos == std::string::npos || descPos < ascPos)) {
                rule.column = utils::strip(token.substr(0, descPos));
                rule.isAscending = false;
            } else if (ascPos != std::string::npos) {
                rule.column = utils::strip(token.substr(0, ascPos));
                rule.isAscending = true;
            } else {
                rule.column = token;
                rule.isAscending = true;
            }
            sortRules.push_back(rule);
        }
        result["order_by"] = SQLVal(sortRules);
    } else {
        result["order_by"] = SQLVal(std::vector<SortRule>());
    }

    // 输出调试信息
    std::cout << "[DEBUG] Parsed result contents:" << std::endl;
    for (const auto& pair : result) {
        std::cout << "[DEBUG]   " << pair.first << ": ";
        if (std::holds_alternative<std::vector<std::string>>(pair.second)) {
            auto vec = std::get<std::vector<std::string>>(pair.second);
            std::cout << "[";
            for (size_t i = 0; i < vec.size(); ++i) {
                std::cout << vec[i];
                if (i < vec.size() - 1) std::cout << ", ";
            }
            std::cout << "]";
        } else if (std::holds_alternative<std::shared_ptr<Node>>(pair.second)) {
            std::cout << "(WHERE tree pointer)";
        } else if (std::holds_alternative<bool>(pair.second)) {
            std::cout << std::get<bool>(pair.second);
        }  else if (std::holds_alternative<std::string>(pair.second)) {
            std::cout << std::get<std::string>(pair.second);
        } else {
            std::cout << "(unknown type)";
        }
        std::cout << std::endl;
    }



// =======
//         R"(SELECT\s+(\*|[\w\s,]+)\s+FROM\s+(\w+)(?:\s+WHERE\s+([^\n;]+))?(?:\s+ORDER\s+BY\s+([^\n;]+))?)",
//         std::regex::icase);
//     std::smatch match;

//     if (std::regex_search(sql, match, pattern)) {
//         result["status"] = true;

//         // 解析列名（支持 *）
//         std::string columnsStr = utils::strip(match[1].str());
//         std::vector<std::string> columns;
//         if (columnsStr == "*") {
//             columns = {}; // 空数组代表 SELECT *
//         } else {
//             std::vector<std::string> rawColumns = utils::split(columnsStr, ",");
//             for (auto& col : rawColumns) {
//                 col = utils::strip(col);
//                 if (!col.empty()) {
//                     columns.push_back(col);
//                 }
//             }
//         }
//         result["columns"] = columns;

//         // 解析表名
//         result["table"] = utils::strip(match[2].str());

//         // 记录日志
//         Logger logger("../../res/system_logs.txt");
//         logger.log(Session::getCurrentUserId(), "SELECT", "TABLE", std::get<std::string>(result["table"]) + " from " + dbName); // 记录日志

//         // 解析 WHERE 子句
//         if (match[3].matched) {
//             std::string whereStr = utils::strip(match[3].str());
//             result["whereTree"] = parsWhereClause(whereStr);
//         }

//         // 解析 ORDER BY 子句
//         if (match[4].matched) {
//             std::vector<SortRule> sortRules;
//             std::string orderByStr = match[4].str();
//             std::vector<std::string> orderTokens = utils::split(orderByStr, ",");
//             for (auto& token : orderTokens) {
//                 token = utils::strip(token);
//                 if (token.empty()) continue;

//                 SortRule rule;
//                 size_t spacePos = token.find_last_of(" \t"); // 查找排序关键字
//                 if (spacePos != std::string::npos) {
//                     std::string col = token.substr(0, spacePos);
//                     std::string order = token.substr(spacePos + 1);
//                     rule.column = utils::strip(col);
//                     rule.isAscending = (order == "ASC" || order.empty());
//                 } else {
//                     rule.column = token;
//                     rule.isAscending = true; // 默认升序
//                 }
//                 sortRules.push_back(rule);
//             }
//             result["order_by"] = sortRules;
//         }
//     }
// >>>>>>> 9e3794f9e4ed8e4a413a17e5976ebb9162f7bbed
    return result;
}

// Assume split, utils::strip, utils::toUpper, parsWhereClause, SQLVal definitions exist

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
std::map<std::string, SQLVal> Lexer::parseAlter(const std::string& sql, tableManage& tableMgr) {
    std::map<std::string, SQLVal> result = { {"type", std::string("ALTER")}, {"status", false} };
    std::regex pattern(R"(ALTER\s+TABLE\s+(\w+)\s+(ADD|MODIFY|DROP)\s+([\s\S]*))", std::regex::icase);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = true;
        result["table"] = std::string(match[1].str());
        result["action"] = std::string(match[2].str());
        std::string columnsStr = std::string(match[3].str());

        std::vector<std::string> fieldNames;
        std::vector<int> fieldOrders;
        std::vector<std::string> fieldTypes;
        std::vector<int> fieldTypeParams;
        std::vector<std::string> constraints;

        std::string action = std::get<std::string>(result["action"]);
        if (action == "ADD") {
            // 原有的 ADD 操作逻辑
            std::regex columnPattern(R"((\w+)\s+([\w\(\)]+)\s*([^,]*))");
            auto columnBegin = std::sregex_iterator(columnsStr.begin(), columnsStr.end(), columnPattern);
            auto columnEnd = std::sregex_iterator();
            for (std::sregex_iterator it = columnBegin; it != columnEnd; ++it) {
                std::string fieldName = utils::strip((*it)[1].str());
                std::string fieldTypeStr = utils::strip((*it)[2].str());
                std::string constraint = utils::strip((*it)[3].str());

                // 假设字段顺序默认为当前字段在列表中的顺序（从1开始）
                int fieldOrder = static_cast<int>(fieldNames.size() + 1);

                // 解析字段类型和参数
                std::regex typeParamPattern(R"((\w+)\((\d+)\))");
                std::smatch typeParamMatch;
                std::string fieldType;
                int fieldTypeParam = 0;
                if (std::regex_search(fieldTypeStr, typeParamMatch, typeParamPattern)) {
                    fieldType = typeParamMatch[1].str();
                    fieldTypeParam = std::stoi(typeParamMatch[2].str());
                } else {
                    fieldType = fieldTypeStr;
                }

                fieldNames.push_back(fieldName);
                fieldOrders.push_back(fieldOrder);
                fieldTypes.push_back(fieldType);
                fieldTypeParams.push_back(fieldTypeParam);
                constraints.push_back(constraint);
            }

            // 在字段添加后记录日志
            Logger logger("../../res/system_logs.txt");
            for (const auto& fieldName : fieldNames) {
                logger.log(Session::getCurrentUserId(), "ADD", "FIELD", fieldName + " to " + std::get<std::string>(result["table"]));
            }

        } else if (action == "DROP") {
            // 原有的 DROP 操作逻辑
            std::regex columnPattern(R"(\w+)");
            auto columnBegin = std::sregex_iterator(columnsStr.begin(), columnsStr.end(), columnPattern);
            auto columnEnd = std::sregex_iterator();
            for (std::sregex_iterator it = columnBegin; it != columnEnd; ++it) {
                std::string fieldName = utils::strip((*it)[0].str());
                fieldNames.push_back(fieldName);
                // 对于删除操作，字段顺序、类型、参数和约束可以忽略
                fieldOrders.push_back(0);
                fieldTypes.push_back("");
                fieldTypeParams.push_back(0);
                constraints.push_back("");

                // 在字段删除后记录日志
                Logger logger("../../res/system_logs.txt");
                logger.log(Session::getCurrentUserId(), "DROP", "FIELD", fieldName + " from " + std::get<std::string>(result["table"]));
            }
        } else if (action == "MODIFY") {
            // 新增的 MODIFY 操作逻辑
            std::regex columnPattern(R"((\w+)\s+([\w\(\)]+)\s*([^,]*))");
            auto columnBegin = std::sregex_iterator(columnsStr.begin(), columnsStr.end(), columnPattern);
            auto columnEnd = std::sregex_iterator();
            for (std::sregex_iterator it = columnBegin; it != columnEnd; ++it) {
                std::string fieldName = utils::strip((*it)[1].str());
                std::string fieldTypeStr = utils::strip((*it)[2].str());
                std::string constraint = utils::strip((*it)[3].str());

                // 解析字段类型和参数
                std::regex typeParamPattern(R"((\w+)\((\d+)\))");
                std::smatch typeParamMatch;
                std::string fieldType;
                int fieldTypeParam = 0;
                if (std::regex_search(fieldTypeStr, typeParamMatch, typeParamPattern)) {
                    fieldType = typeParamMatch[1].str();
                    fieldTypeParam = std::stoi(typeParamMatch[2].str());
                } else {
                    fieldType = fieldTypeStr;
                }

                // 假设字段顺序不变
                int fieldOrder = 0;

                fieldNames.push_back(fieldName);
                fieldOrders.push_back(fieldOrder);
                fieldTypes.push_back(fieldType);
                fieldTypeParams.push_back(fieldTypeParam);
                constraints.push_back(constraint);

                // 在字段修改后记录日志
                Logger logger("../../res/system_logs.txt");
                logger.log(Session::getCurrentUserId(), "MODIFY", "FIELD", fieldName + " in " + std::get<std::string>(result["table"]));
            }
        }

        // 从 std::variant 中提取 std::string 类型的值
        std::string* actionPtr = std::get_if<std::string>(&result["action"]);
        if (actionPtr) {
            // 在 Lexer 中进行增加字段的预处理逻辑
            // 例如，检查字段名是否合法等

            for (const auto& fieldName : fieldNames) {
                if (fieldName.length() > 128) {
                    result["status"] = false;
                    std::cerr << "Invalid field name: " << fieldName << " (length exceeds 128 characters)" << std::endl;
                    return result;
                }
            }

            // 调用 tableManage 的 alterTable 方法更新表信息
            if (tableMgr.alterTable(currentDatabase, std::get<std::string>(result["table"]), *actionPtr,
                                    fieldNames, fieldOrders, fieldTypes, fieldTypeParams, constraints)) {
                result["status"] = true;
                // 发射信号通知表定义已更改
                emit tableDefinitionChanged(QString::fromStdString(std::get<std::string>(result["table"])));
                std::cout << "Altering finished." << std::endl;
            } else {
                result["status"] = false;
                std::cerr << "Altering failed." << std::endl;
            }
        }
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

        // 记录日志
        Logger logger("../../res/system_logs.txt"); // 指定日志文件路径
        logger.log(Session::getCurrentUserId(), "SHOW", std::string(match[1].str()), "requested in " + currentDatabase); // 记录 SHOW 操作的日志


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
    if (affair.isrunning) {
        affair.writeToUndo(QString::fromStdString(sql));
    }
    std::map<std::string, SQLVal> result = { {"type", std::string("UPDATE")}, {"status", false} };
    std::regex pattern(R"(UPDATE\s+(\w+)\s+SET\s+(.+?)\s+WHERE\s+(\w+)\s*=\s*(.+))", ICASE);
    std::smatch match;

    if (std::regex_search(sql, match, pattern)) {
        result["status"] = true;
        result["table"] = match[1].str();

        // 解析 SET 子句
        std::string setClause = match[2].str();
        std::map<std::string, std::string> setMap;

        std::vector<std::string> assignments = utils::split(setClause, ",");
        for (auto& assign : assignments) {
            std::vector<std::string> pair = utils::split(assign, "=");
            if (pair.size() == 2) {
                std::string key = utils::strip(pair[0]);
                std::string value = utils::strip(pair[1]);
                setMap[key] = value;
            }
        }
        result["set"] = SQLVal(setMap);

        result["condition_column"] = match[3].str();
        result["condition_value"] = match[4].str();
    }

    // 日志记录更新操作
    Logger logger("../../res/system_logs.txt");
    std::string tableName;

    // 安全地从 variant 中获取字符串值
    if (auto tableVar = std::get_if<std::string>(&result["table"])) {
        tableName = *tableVar;
    } else {
        tableName = "unknown_table"; // 默认值，防止崩溃
    }

    logger.log(Session::getCurrentUserId(), "UPDATE", "TABLE",
               tableName + " in " + currentDatabase); // 记录日志

    return result;
}




/**Test Finished
 * Wait for examination
 * TODO:Need an extra function to parse the condition
*/
std::map<std::string, SQLVal> Lexer::parseDelete(const std::string& sql) {
    if (affair.isrunning) {
        affair.writeToUndo(QString::fromStdString(sql));
    }
    std::map<std::string, SQLVal> result = { {"type", std::string("DELETE")}, {"status", false} };
    std::regex pattern_where(R"(DELETE\s+FROM\s+(\w+)\s+WHERE\s+(\w+)\s*=\s*(\w+))", ICASE);
    std::regex pattern_without_where(R"(DELETE\s+FROM\s+(\w+)\s*;?)", ICASE);

    std::smatch match;

    if (std::regex_search(sql, match, pattern_where)) {
        result["status"] = true;
        result["table"] = match[1].str();
        result["key"] = match[2].str();
        result["value"] = match[3].str();  // 这里是主键的值

        // 日志记录删除操作
        Logger logger("../../res/system_logs.txt");
        std::string tableName;

        // 安全地从 variant 中获取字符串值
        if (auto tableVar = std::get_if<std::string>(&result["table"])) {
            tableName = *tableVar;
        } else {
            tableName = "unknown_table"; // 默认值，防止崩溃
        }

        logger.log(Session::getCurrentUserId(), "DELETE", "TABLE",
                   tableName + " from " + currentDatabase); // 记录日志
    }

    else if (std::regex_search(sql, match, pattern_without_where)){
        result["status"] = true;
        result["table"] = match[1].str();
        result["key"] = "ALL";     // 标记为删除整表
        result["value"] = "ALL";   // 无特定值
        std::cout << "[DEBUG] DELETE entire table: " << match[1].str() << std::endl;

    }

    return result;
}
/**Test Finished
 * Wait for examination
*/
std::map<std::string, SQLVal> Lexer::parseDescribe(const std::string& sql) {
    std::map<std::string, SQLVal> result = { {"type", std::string("DESCRIBE")}, {"status", false} };
    std::regex pattern(R"(DESCRIBE\s+(\w+)(?:\s+(\w+))?)", ICASE);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = true;
        result["table"] = std::string(match[1]);
        if (match[2].matched) result["column"] = match[2];
    }
    return result;
}


std::map<std::string, SQLVal> Lexer::parseStart(const std::string& sql) {
    std::map<std::string, SQLVal> result = { {"type", std::string("START")}, {"status", false} };
    std::regex pattern(R"(START\s+TRANSACTION)", std::regex::icase);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = true;
        affair.start();
        Logger logger("../../res/system_logs.txt");
        logger.log(Session::getCurrentUserId(), "START", "TRANSACTION", "Started transaction");
    }
    return result;
}
std::map<std::string, SQLVal> Lexer::parseRollback(const std::string& sql) {
    std::map<std::string, SQLVal> result = { {"type", std::string("ROLLBACK")}, {"status", false} };
    std::regex pattern(R"(ROLLBACK)", std::regex::icase);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = true;
        affair.rollback();
        affair.recover();
        Logger logger("../../res/system_logs.txt");
        logger.log(Session::getCurrentUserId(), "ROLLBACK", "TRANSACTION", "Rolled back transaction");
    }
    return result;
}

std::map<std::string, SQLVal> Lexer::parseCommit(const std::string& sql) {
    std::map<std::string, SQLVal> result = { {"type", std::string("COMMIT")}, {"status", false} };
    std::regex pattern(R"(COMMIT)", std::regex::icase);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = true;
        affair.commit();
        Logger logger("../../res/system_logs.txt");
        logger.log(Session::getCurrentUserId(), "COMMIT", "TRANSACTION", "Committed transaction");
    }
    return result;
}


std::map<std::string, SQLVal> Lexer::parseRecover(const std::string& sql) {
    std::map<std::string, SQLVal> result = { {"type", std::string("RECOVER")}, {"status", false} };
    std::regex pattern(R"(RECOVER)", std::regex::icase);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = true;
        affair.recover();
        Logger logger("../../res/system_logs.txt");
        logger.log(Session::getCurrentUserId(), "RECOVER", "TRANSACTION", "Recovered transaction");
    }
    return result;
}

using ParseFunc = std::function<std::map<std::string, SQLVal>(const std::string&)>;

std::map<std::string, SQLVal> Lexer::parseSQL(const std::string& sql) {
    std::vector<std::pair<std::regex, ParseFunc>> patterns = {
        {std::regex(R"(^CREATE\s)", ICASE), [this](const std::string& sql) { return parseCreate(sql); }},
      {std::regex(R"(^INSERT\s)", ICASE), [this](const std::string& sql) { return parseInsert(sql); }},
      {std::regex(R"(^SELECT\s)", ICASE), [this](const std::string& sql) { return parseSelect(sql); }},
      {std::regex(R"(^USE\s)", ICASE), [this](const std::string& sql) { return parseUse(sql); }},
      {std::regex(R"(^DROP\s)", ICASE), [this](const std::string& sql) { return parseDrop(sql); }},
      {std::regex(R"(^GRANT\s)", ICASE), [this](const std::string& sql) { return parseGrant(sql); }},
      {std::regex(R"(^REVOKE\s)", ICASE), [this](const std::string& sql) { return parseRevoke(sql); }},
      {std::regex(R"(^ALTER\s)", ICASE), [this](const std::string& sql) { return parseAlter(sql, tableMgr); }},
      {std::regex(R"(^SHOW\s)", ICASE), [this](const std::string& sql) { return parseShow(sql); }},
      {std::regex(R"(^UPDATE\s)", ICASE), [this](const std::string& sql) { return parseUpdate(sql); }},
      {std::regex(R"(^START\s)", ICASE), [this](const std::string& sql) { return parseStart(sql); }},
      {std::regex(R"(^ROLLBACK\s)", ICASE), [this](const std::string& sql) { return parseRollback(sql); }},
      {std::regex(R"(^COMMIT\s)", ICASE), [this](const std::string& sql) { return parseCommit(sql); }},
      {std::regex(R"(^DELETE\s)", ICASE), [this](const std::string& sql) { return parseDelete(sql); }},
      {std::regex(R"(^DESCRIBE\s)", ICASE), [this](const std::string& sql) { return parseDescribe(sql); }},
        //{std::regex(R"(^RECOVER\s)", ICASE), [this](const std::string& sql) { return parseRecover(sql); }},
        {std::regex(R"(^START\s+TRANSACTION\s*)", std::regex::icase), [this](const std::string& sql) { return parseStart(sql); }},
        {std::regex(R"(^ROLLBACK\s*)", std::regex::icase), [this](const std::string& sql) { return parseRollback(sql); }},
        {std::regex(R"(^COMMIT\s*)", std::regex::icase), [this](const std::string& sql) { return parseCommit(sql); }},
        {std::regex(R"(^RECOVER\s*)", std::regex::icase), [this](const std::string& sql) { return parseRecover(sql); }}
    };

    for (const auto& [pattern, func] : patterns) {
        if (std::regex_search(sql, pattern)) {
            return func(sql);
        }
    }
    Wrong::getInstance("syntax error")->show();
    return { {"type", std::string("UNKNOWN")}, {"status", false} };
}


void Lexer::handleRawSQL(QString rawSql){
    std::vector<std::string> sqls=utils::split(rawSql.toStdString(),";");
    for(std::string sql:sqls){
        std::map<std::string,SQLVal> result = parseSQL(sql);
        std::string type = std::get<std::string>(result["type"]);
        bool status = std::get<bool>(result["status"]);
        std::cout << "Type: " << type << ", Status: " << status << std::endl;

        if (!status) continue;

        if (type == "INSERT") {
            std::string tableName = std::get<std::string>(result["table"]);
            std::vector<std::vector<std::string>> valuesGroups =
                std::get<std::vector<std::vector<std::string>>>(result["values"]);

            std::string dbName = getCurrentDatabase();  // 从 Lexer 获取当前数据库
            datamanager dataManager(&dbMgr);                    // 实例化 datamanager（注意命名一致）
            for (const auto& values : valuesGroups) {
                dataManager.insertData(dbName, tableName, values);
                std::cout << "[DEBUG] dbName: " << dbName << ", tableName: " << tableName << std::endl;
            }
        }





        if (type == "UPDATE") {
            std::string tableName = std::get<std::string>(result["table"]);
            auto setMap = std::get<std::map<std::string, std::string>>(result["set"]);
            std::string pkName = std::get<std::string>(result["condition_column"]);
            std::string pkValue = std::get<std::string>(result["condition_value"]);

            std::string dbName = getCurrentDatabase();
            datamanager dataManager(&dbMgr);
            dataManager.updateData(dbName, tableName, setMap, pkName, pkValue);
        }


        if (type == "DELETE") {
            std::string dbName = getCurrentDatabase();
            std::string tableName = std::get<std::string>(result["table"]);
            std::string key = std::get<std::string>(result["key"]);
            std::string value = std::get<std::string>(result["value"]);

            std::cout << "[DEBUG] DELETE SQL: dbName = " << dbName
                      << ", table = " << tableName
                      << ", key = " << key
                      << ", value = " << value << std::endl;

            // 获取字段信息，判断主键
            fieldManage fieldMgr;
            std::vector<fieldManage::FieldInfo> fields = fieldMgr.getFieldsInfo(dbName, tableName);

            if (fields.empty()) {
                std::cerr << "[ERROR] Field info is empty. Possibly failed to read table definition file." << std::endl;
                return;
            }

            std::cout << "[DEBUG] Fields loaded from .tdf.txt:" << std::endl;
            for (const auto& field : fields) {
                std::cout << "  FieldName: " << field.fieldName
                          << ", Constraints: " << field.constraints << std::endl;
            }

            std::string primaryKey;
            for (const auto& field : fields) {
                std::string constraintLower = field.constraints;
                std::transform(constraintLower.begin(), constraintLower.end(), constraintLower.begin(), ::tolower);
                if (constraintLower.find("primary key") != std::string::npos) {
                    primaryKey = field.fieldName;
                    break;
                }
            }

            std::cout << "[DEBUG] Detected primary key: " << primaryKey << std::endl;

            if (primaryKey.empty()) {
                std::cerr << "Error: No primary key found in table '" << tableName << "'." << std::endl;
                return;
            }

            //整表删除时 跳过主键验证
            if (key == "ALL") {
                std::cout << "[DEBUG] Entire table delete, bypassing primary key check." << std::endl;
            } else if (key != primaryKey) {
                std::cerr << "Error: DELETE statement must use the primary key column. Expected key: "
                          << primaryKey << ", but got: " << key << std::endl;
                return;
            }

            // if (key != primaryKey) {
            //     std::cerr << "Error: DELETE statement must use the primary key column. Expected key: " << primaryKey << ", but got: " << key << std::endl;
            //     return;
            // }

            datamanager dataManager(&dbMgr);
            dataManager.deleteData(dbName, tableName, value);
        }

        if (type == "SELECT") {
            // 使用安全的方式获取各参数
            std::string tableName;
            if (std::holds_alternative<std::string>(result["table"])) {
                tableName = std::get<std::string>(result["table"]);
            } else {
                std::cerr << "[ERROR] Invalid type for table name!" << std::endl;
                return;
            }

            std::vector<std::string> columns;
            if (result.find("columns") != result.end() &&
                std::holds_alternative<std::vector<std::string>>(result["columns"])) {
                columns = std::get<std::vector<std::string>>(result["columns"]);
            } else {
                std::cout << "[DEBUG] No columns specified, using empty vector" << std::endl;
                // 允许 columns 为空，只要有聚合函数
            }

            std::vector<std::string> aggregateFunctions;
            if (result.find("aggregates") != result.end() &&
                std::holds_alternative<std::vector<std::string>>(result["aggregates"])) {
                aggregateFunctions = std::get<std::vector<std::string>>(result["aggregates"]);
            }

            std::vector<std::string> groupByColumns;
            if (result.find("group_by") != result.end() &&
                std::holds_alternative<std::vector<std::string>>(result["group_by"])) {
                groupByColumns = std::get<std::vector<std::string>>(result["group_by"]);
            }




            // 如果没有普通列但有聚合函数，这是合法的
            if (columns.empty() && aggregateFunctions.empty()) {
                std::cerr << "[ERROR] Query must specify columns or aggregates!" << std::endl;
                return;
            }

            // // 这里修改对 columns 为空的判断逻辑
            // if (columns.empty() && aggregateFunctions.empty()) {
            //     // 检查是否是 SELECT * 情况（即 columns 里有特殊标识 "ALL_COLUMNS" ）
            //     if (std::find(columns.begin(), columns.end(), "ALL_COLUMNS") == columns.end()) {
            //         std::cerr << "[ERROR] Query must specify columns or aggregates!" << std::endl;
            //         return;
            //     }
            // }

            std::shared_ptr<Node> whereTree = nullptr;
            if (result.find("whereTree") != result.end() &&
                std::holds_alternative<std::shared_ptr<Node>>(result["whereTree"])) {
                whereTree = std::get<std::shared_ptr<Node>>(result["whereTree"]);
            }

            std::string dbName = getCurrentDatabase();
            datamanager dataManager(&dbMgr);

            std::cout << "[DEBUG] Checking result types before execution:" << std::endl;
            std::cout << "[DEBUG]   table type: " << (std::holds_alternative<std::string>(result["table"]) ? "string" : "other") << std::endl;
            std::cout << "[DEBUG]   columns type: " << (std::holds_alternative<std::vector<std::string>>(result["columns"]) ? "vector<string>" : "other") << std::endl;
            std::cout << "[DEBUG]   aggregates type: " << (result.count("aggregates") && std::holds_alternative<std::vector<std::string>>(result["aggregates"]) ? "vector<string>" : "other") << std::endl;
            std::cout << "[DEBUG]   group_by type: " << (result.count("group_by") && std::holds_alternative<std::vector<std::string>>(result["group_by"]) ? "vector<string>" : "other") << std::endl;
            std::cout << "[DEBUG]   whereTree type: " << (result.count("whereTree") && std::holds_alternative<std::shared_ptr<Node>>(result["whereTree"]) ? "shared_ptr<Node>" : "other") << std::endl;



            // 调用执行器
            std::vector<std::vector<std::string>> rows = dataManager.selecData(
                dbName, tableName, columns, whereTree, aggregateFunctions, groupByColumns
                );

            std::cout << "[DEBUG] SELECT Result (" << rows.size() << " rows):\n";
            for (const auto& row : rows) {
                for (const auto& val : row) {
                    std::cout << val << "\t";
                }
                std::cout << std::endl;
            }

            std::string output;

            // 加入表头
            if (!columns.empty()) {
                for (size_t i = 0; i < columns.size(); ++i) {
                    output += columns[i];
                    if (i < columns.size() - 1)
                        output += "\t";
                }
                output += "\n";
            }

            // 加入每一行数据
            for (const auto& row : rows) {
                for (size_t i = 0; i < row.size(); ++i) {
                    output += row[i];
                    if (i < row.size() - 1)
                        output += "\t";
                }
                output += "\n";
            }

            emit sendSelectResult(rows);
    }
        if(status){
            Server * server = Server::getInstance();
            server->sendMessageToServer(QString::fromStdString(sql));
        }
}
}

void Lexer::setCurrentDatabase(const std::string& dbName) {
    currentDatabase = dbName;
}

void Lexer::setCurrentTable(const std::string& tableName) {
    currentTable = tableName;
}

std::string Lexer::getCurrentDatabase()const{
    return currentDatabase;
}

std::vector<std::string> Lexer::split(const std::string& s, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0, end = 0;
    while ((end = s.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + delimiter.length();
    }
    tokens.push_back(s.substr(start));
    return tokens;
}

