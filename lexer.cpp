#include "lexer.h"
#include "utils.h"
#include <iostream>
#include <regex>
#include <map>
#include <vector>
#include <variant>
#include <memory>
#include <utility>

Lexer::Lexer() {}

struct Condition;
struct LogicalOp;

using Node = std::variant<Condition, LogicalOp>;

struct Condition {
    std::string column;
    std::string op;
    std::string value;
};

struct LogicalOp {
    std::string op; // "AND" 或 "OR"
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;
};

#define ICASE std::regex_constants::icase

/*std::string serializeColumns(const std::vector<std::map<std::string, std::string>>& columns) {
    std::string result;
    for (const auto& column : columns) {
        result += column.at("name") + " " + column.at("type");
        if (!column.at("constraints").empty()) {
            result += " " + column.at("constraints");
        }
        result += ";";
    }
    return result;
}*/
using SQLVal = std::variant< bool, std::string, std::vector<std::string>,
                            std::vector<std::map<std::string,std::string>>,
                            std::vector<std::vector<std::string>>,std::shared_ptr<Node> >;

std::shared_ptr<Node> parseWhereClause(const std::string& whereStr);

// 解析单个条件
std::shared_ptr<Node> parseCondition(const std::string& conditionStr) {
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
std::shared_ptr<Node> parseLogicalOp(const std::string& logicalStr) {
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
std::shared_ptr<Node> parseWhereClause(const std::string& whereStr) {
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

/**Test Finished 
 * Wait for examination
*/
std::map<std::string, SQLVal> parseCreate(const std::string& sql) {
    std::map<std::string,SQLVal> result = { {"type", std::string("CREATE")}, {"status", false} };

    std::regex createPattern(R"(CREATE\s+(DATABASE|TABLE)\s+(\w+)\s*(?:\(([\s\S]*?)\))?\s*;$)", ICASE);
    std::smatch match;

    if (std::regex_search(sql, match, createPattern)) {
        result["status"] = true;
        result["object_type"] = std::string(match[1].str());
        result["object_name"] = std::string(match[2].str());

        if (std::get<std::string>(result["object_type"]) == std::string("TABLE") && match[3].matched) {
            std::string columnsStr = match[3];
            std::regex columnPattern(R"(\s*(\w+)\s+(\w+)\s*([\s\S]*?)\s*(?:,|$))");
            std::smatch columnMatch;
            std::vector<std::map<std::string, std::string>> columns;

            auto columnBegin = std::sregex_iterator(columnsStr.begin(), columnsStr.end(), columnPattern);
            auto columnEnd = std::sregex_iterator();

            for (std::sregex_iterator it = columnBegin; it != columnEnd; ++it) {
                std::map<std::string, std::string> column;
                column["name"] = (*it)[1].str();
                column["type"] = (*it)[2].str();
                column["constraints"] = (*it)[3].str();
                columns.push_back(column);
            }
            result["columns"] = columns;
        }
    }
    return result;
}
/**Test Finished 
 * Wait for examination
*/
std::map<std::string,SQLVal> parseDrop(const std::string& sql){
    std::map<std::string,SQLVal> result = { {"type",std::string("DROP")}, {"status",false}, {"restrict",true} };
    std::regex pattern(R"(DROP\s(DATABASE|TABLE)\s(\w+)\s*(RESTRICT|CASCADE)*;$)",ICASE);
    std::smatch match;
    if(std::regex_search(sql,match,pattern)){
        result["status"]=true;
        result["object_type"]=std::string(match[1].str());
        result["object_name"]=std::string(match[2].str());
        
        if (match[3].matched) {
            result["restrict"] = (match[3].str() == "RESTRICT");
        }
    }
    return result;
}
/**Test Finished 
 * Wait for examination
*/
std::map<std::string, SQLVal> parseInsert(const std::string& sql) {
    std::map<std::string, SQLVal> result = { {"type", "INSERT"}, {"status", false} };

    std::regex pattern(R"(^INSERT\s+INTO\s+(\w+)\s*\(([^)]+)\)\s*VALUES\s*(\([\S\s]+\));?$)", ICASE);
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

/**Test Finished 
 * Wait for examination
*/
std::map<std::string,SQLVal> parseUse(const std::string& sql){
    std::map<std::string, SQLVal> result = { {"type", std::string("USE")}, {"status", false} };
    std::regex pattern(R"(USE\s+(\w)+;$)",ICASE);
    std::smatch match;

    if(std::regex_search(sql,match,pattern)){
        result["status"] = true;
        result["name"] = std::string(match[1].str());
    }
    return result;
}

/**Test Finished 
 * Wait for examination
 * TODO:Need to parse "AS" and conditions
*/
std::map<std::string, SQLVal> parseSelect(const std::string& sql) {
    std::map<std::string, SQLVal> result = { {"type", std::string("SELECT")}, {"status", false} };
    std::regex pattern(R"(SELECT\s+(.*?)\s+FROM\s+(\w+)(?:\s+WHERE\s+(.*?))?(?:\s+GROUP\s+BY\s+(.*?))?(?:\s+HAVING\s+(.*?))?(?:\s+ORDER\s+BY\s+(.*?))?(?:\s+LIMIT\s+(.*?))?;$)", ICASE);
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

/**Test Finished 
 * Wait for examination
*/
std::map<std::string, SQLVal> parseGrant(const std::string& sql) {
    std::map<std::string, SQLVal> result = { {"type", std::string("GRANT")}, {"status", false} };
    std::regex pattern(R"(GRANT\s+([\w,\s]+)\s+ON\s+(\w+)\s+TO\s+(\w+);$)", ICASE);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = true;
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
std::map<std::string, SQLVal> parseRevoke(const std::string& sql) {
    std::map<std::string, SQLVal> result = { {"type", std::string("REVOKE")}, {"status", false} };
    std::regex pattern(R"(REVOKE\s+([\w,\s]+)\s+ON\s+(\w+)\s+FROM\s+(\w+);$)", ICASE);
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
std::map<std::string, SQLVal> parseAlter(const std::string& sql) {
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

/**Test Finished 
 * Wait for examination
*/
std::map<std::string, SQLVal> parseShow(const std::string& sql) {
    std::map<std::string, SQLVal> result = { {"type", std::string("SHOW")}, {"status", false} };
    std::regex pattern(R"(SHOW\s+(TABLES|DATABASES)(?:\s+FROM\s+(\w+))?;$)", ICASE);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = true;
        result["item"] = std::string(match[1].str());
        if (match[2].matched) result["database"] = match[2];
    }
    return result;
}

/**Test Finished 
 * Wait for examination
*/
std::map<std::string, SQLVal> parseUpdate(const std::string& sql) {
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
std::map<std::string, SQLVal> parseDelete(const std::string& sql) {
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
std::map<std::string, SQLVal> parseDescribe(const std::string& sql) {
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

using ParseFunc = std::map<std::string, SQLVal>(*)(const std::string&);
std::map<std::string, SQLVal> parseSQL(const std::string& sql) {
    std::vector<std::pair<std::regex, ParseFunc>> patterns = {
        {std::regex(R"(^CREATE\s)", ICASE), parseCreate},
        {std::regex(R"(^INSERT\s)", ICASE), parseInsert},
        {std::regex(R"(^SELECT\s)", ICASE), parseSelect},
        {std::regex(R"(^USE\s)", ICASE), parseUse},
        {std::regex(R"(^DROP\s)", ICASE), parseDrop},
        {std::regex(R"(^GRANT\s)", ICASE), parseGrant},
        {std::regex(R"(^REVOKE\s)", ICASE), parseRevoke},
        {std::regex(R"(^ALTER\s)", ICASE), parseAlter},
        {std::regex(R"(^SHOW\s)", ICASE), parseShow},
        {std::regex(R"(^UPDATE\s)", ICASE), parseUpdate},
        {std::regex(R"(^DELETE\s)", ICASE), parseDelete},
        {std::regex(R"(^DESCRIBE\s)", ICASE), parseDescribe}
    };
    for (const auto& [pattern, func] : patterns) {
        if (std::regex_search(sql, pattern)) {
            return func(sql);
        }
    }
    return { {"type", std::string("UNKNOWN")}, {"status", false} };
}

void Lexer::handleRawSQL(QString rawSql){
    std::vector<std::string> sqls=utils::split(rawSql.toStdString(),";");
    for(std::string sql:sqls){
        std::map<std::string,SQLVal> result = parseSQL(sql);
        std::cout << "Type: " << std::get<std::string>(result["type"]) << ", Status: " << std::get<bool>(result["status"]) << std::endl;
    }
}

