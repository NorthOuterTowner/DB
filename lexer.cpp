#include "lexer.h"
#include <iostream>
#include <regex>
#include <map>
#include <vector>
#include <QDebug>
#include <tuple>

Lexer::Lexer() {}
/**
 * @brief: SerializeColumns when needed. But In fact,
 * most of times, it won't be used because deliver columns 
 * to function which deal with discrete logic as paramters 
 * is the best way to use it. 
 */
std::string serializeColumns(const std::vector<std::map<std::string, std::string>>& columns) {
    std::string result;
    for (const auto& column : columns) {
        result += column.at("name") + " " + column.at("type");
        if (!column.at("constraints").empty()) {
            result += " " + column.at("constraints");
        }
        result += ";";
    }
    return result;
}
/**
 * @brief: Parsing CREATE SQL and return relative info.
 * @param: SQL.
 * @return:A map named "result" which has four index: 
 *         - "status":whether grammer is correct,
 *         - "type":CREATE(fixed structure),
 *         - "object_type": DATABASE/TABLE,
 *         - "object_name": The name of database or table;
 */
std::map<std::string, std::string> parseCreate(const std::string& sql) {
    std::map<std::string, std::string> result;
    result["type"] = "CREATE";
    result["status"] = "false";

    std::regex createPattern(R"(CREATE\s+(DATABASE|TABLE)\s+(\w+)\s*(?:\(([\s\S]*?)\))?\s*;)", std::regex_constants::icase);
    std::smatch match;

    if (std::regex_search(sql, match, createPattern)) {
        result["status"] = "true";
        result["object_type"] = match[1];
        result["object_name"] = match[2];

        if (result["object_type"] == "TABLE" && match[3].matched) {
            std::string columnsStr = match[3];
            std::regex columnPattern(R"(\s*(\w+)\s+(\w+)\s*([\s\S]*?)\s*(?:,|$))");
            std::smatch columnMatch;
            std::vector<std::map<std::string, std::string>> columns;

            auto columnBegin = std::sregex_iterator(columnsStr.begin(), columnsStr.end(), columnPattern);
            auto columnEnd = std::sregex_iterator();

            for (std::sregex_iterator it = columnBegin; it != columnEnd; ++it) {
                std::map<std::string, std::string> column;
                column["name"] = (*it)[1];
                column["type"] = (*it)[2];
                column["constraints"] = (*it)[3];
                columns.push_back(column);
            }

            if (!columns.empty()) {
                result["columns"] = serializeColumns(columns);
                std::cout << "Columns parsed successfully." << std::endl;
                std::cout<<result["columns"]<<std::endl;
            }
        }
    }

    std::cout<<result["object_type"] << "\t" << result["object_name"];
    return result;
}

/**
 * @brief: Parsing DROP SQL and return relative info.
 * @param: SQL.
 * @return:A map named "result" which has four index: 
 *         - "status":whether grammer is correct,
 *         - "type":DROP(fixed structure),
 *         - "object_type": DATABASE/TABLE,
 *         - "object_name": The name of database or table;
 */
std::map<std::string,std::string> parseDrop(const std::string& sql){
    std::map<std::string,std::string> result;
    result["type"] = "DROP";
    result["status"] = "false";
    result["mode"] = "restrict";
    std::regex pattern(R"(DROP\s(DATABASE|TABLE)\s(\w+)\s*(RESTRICT|CASCADE)*;)",std::regex_constants::icase);
    std::smatch match;
    if(std::regex_search(sql,match,pattern)){
        result["status"]="true";
        result["object_type"]=match[1];
        result["object_name"]=match[2];
    }
    return result;
}

/**
 * @brief:Parsing INSERT SQL and return relative info.
 * @param: SQL.
 * @return:A map named "result" which has four index: 
 *         - "status":whether grammer is correct,
 *         - "type":INSERT(fixed structure),
 *         - "columns": column name,
 *         - "values": insert value;
 */
std::map<std::string, std::string> parseInsert(const std::string& sql) {
    std::map<std::string, std::string> result;
    result["type"] = "INSERT";
    result["status"] = "false";
    std::regex pattern(R"()", std::regex_constants::icase);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = "true";
        result["table"] = match[1];
        result["columns"] = match[2];
        result["values"] = match[3];
    }
    return result;
}
/**
 * @brief:Parsing USE SQL and return relative info.
 * @param: SQL.
 * @return:A map named "result" which has four index: 
 *         - "status":whether grammer is correct,
 *         - "type":USE(fixed structure),
 *         - "name":the name of database which will be used
 */
std::map<std::string,std::string> parseUse(const std::string& sql){
    std::map<std::string, std::string> result;
    result["type"] = "USE";
    result["status"] = "false";
    
    std::regex pattern(R"(USE\s+(\w)+;)",std::regex_constants::icase);
    std::smatch match;

    if(std::regex_search(sql,match,pattern)){
        result["status"]="true";
        result["name"] = match[1];
    }
    return result;
}
/**
 * @brief:Parsing SELECT SQL and return relative info.
 */
std::map<std::string, std::string> parseSelect(const std::string& sql) {
    std::map<std::string, std::string> result = { {"type", "SELECT"}, {"status", "false"} };
    std::regex pattern(R"(SELECT\s+(.*?)\s+FROM\s+(\w+)(?:\s+WHERE\s+(.*?))?(?:\s+GROUP\s+BY\s+(.*?))?(?:\s+HAVING\s+(.*?))?(?:\s+ORDER\s+BY\s+(.*?))?(?:\s+LIMIT\s+(.*?))?)", std::regex_constants::icase);
    std::smatch match;

    if (std::regex_search(sql, match, pattern)) {
        result["status"] = "true";

        // 解析列，按逗号拆分
        std::regex colRegex(R"(\w+(?:\s+AS\s+\w+)?)", std::regex_constants::icase);
        std::string matchStr = match[1].str();
        auto colBegin = std::sregex_iterator(matchStr.begin(), matchStr.end(), colRegex);
        auto colEnd = std::sregex_iterator();
        std::string columns;
        for (auto it = colBegin; it != colEnd; ++it) {
            columns += it->str() + ",";
        }
        if (!columns.empty()) columns.pop_back();
        result["columns"] = columns;

        result["table"] = match[2];
        if (match[3].matched) result["where"] = match[3];
        if (match[4].matched) result["group_by"] = match[4];
        if (match[5].matched) result["having"] = match[5];
        if (match[6].matched) result["order_by"] = match[6];
        if (match[7].matched) result["limit"] = match[7];
    }
    std::cout << result["status"] << std::endl;
    return result;
}

std::map<std::string, std::string> parseGrant(const std::string& sql) {
    std::map<std::string, std::string> result = { {"type", "GRANT"}, {"status", "false"} };
    std::regex pattern(R"(GRANT\s+([\w,\s]+)\s+ON\s+(\w+)\s+TO\s+(\w+))", std::regex_constants::icase);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = "true";
        result["rights"] = match[1];
        result["object"] = match[2];
        result["user"] = match[3];
        // 拆分多个权限
        std::vector<std::string> rightsList;
        std::regex rightsRegex(R"(\w+)");
        std::string matchStr = match[1].str();
        auto rightsBegin = std::sregex_iterator(matchStr.begin(), matchStr.end(), rightsRegex);
        auto rightsEnd = std::sregex_iterator();
        for (auto it = rightsBegin; it != rightsEnd; ++it) {
            rightsList.push_back(it->str());
        }
        result["rights_list"] = "";
        for (const auto& right : rightsList) {
            result["rights_list"] += (right + ",");
        }
        if (!result["rights_list"].empty()) result["rights_list"].pop_back();
    }
    return result;
}

// 完善的REVOKE解析，支持多个权限
std::map<std::string, std::string> parseRevoke(const std::string& sql) {
    std::map<std::string, std::string> result = { {"type", "REVOKE"}, {"status", "false"} };
    std::regex pattern(R"(REVOKE\s+([\w,\s]+)\s+ON\s+(\w+)\s+FROM\s+(\w+))", std::regex_constants::icase);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = "true";
        result["rights"] = match[1];
        result["object"] = match[2];
        result["user"] = match[3];
        // 拆分多个权限
        std::vector<std::string> rightsList;
        std::regex rightsRegex(R"(\w+)");
        std::string matchStr = match[1].str();
        auto rightsBegin = std::sregex_iterator(matchStr.begin(), matchStr.end(), rightsRegex);
        auto rightsEnd = std::sregex_iterator();
        for (auto it = rightsBegin; it != rightsEnd; ++it) {
            rightsList.push_back(it->str());
        }
        result["rights_list"] = "";
        for (const auto& right : rightsList) {
            result["rights_list"] += (right + ",");
        }
        if (!result["rights_list"].empty()) result["rights_list"].pop_back();
    }
    return result;
}
std::map<std::string, std::string> parseAlter(const std::string& sql) {
    std::map<std::string, std::string> result = { {"type", "ALTER"}, {"status", "false"} };
    std::regex pattern(R"(ALTER\s+TABLE\s+(\w+)\s+(ADD|MODIFY|DROP)\s+(\w+))", std::regex_constants::icase);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = "true";
        result["table"] = match[1];
        result["action"] = match[2];
        result["column"] = match[3];
    }
    return result;
}

// 完善的SHOW解析，支持SHOW TABLES/DATABASES等
std::map<std::string, std::string> parseShow(const std::string& sql) {
    std::map<std::string, std::string> result = { {"type", "SHOW"}, {"status", "false"} };
    std::regex pattern(R"(SHOW\s+(TABLES|DATABASES)(?:\s+FROM\s+(\w+))?)", std::regex_constants::icase);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = "true";
        result["item"] = match[1];
        if (match[2].matched) result["database"] = match[2];
    }
    return result;
}

// 完善的UPDATE解析，支持WHERE条件
std::map<std::string, std::string> parseUpdate(const std::string& sql) {
    std::map<std::string, std::string> result = { {"type", "UPDATE"}, {"status", "false"} };
    std::regex pattern(R"(UPDATE\s+(\w+)\s+SET\s+(\w+)\s*=\s*(\w+)(?:\s+WHERE\s+(.+))?)", std::regex_constants::icase);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = "true";
        result["table"] = match[1];
        result["column"] = match[2];
        result["value"] = match[3];
        if (match[4].matched) result["condition"] = match[4];
    }
    return result;
}

// 完善的DELETE解析，支持WHERE条件
std::map<std::string, std::string> parseDelete(const std::string& sql) {
    std::map<std::string, std::string> result = { {"type", "DELETE"}, {"status", "false"} };
    std::regex pattern(R"(DELETE\s+FROM\s+(\w+)(?:\s+WHERE\s+(.+))?)", std::regex_constants::icase);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = "true";
        result["table"] = match[1];
        if (match[2].matched) result["condition"] = match[2];
    }
    return result;
}

// 完善的DESCRIBE解析，支持DESCRIBE table column形式
std::map<std::string, std::string> parseDescribe(const std::string& sql) {
    std::map<std::string, std::string> result = { {"type", "DESCRIBE"}, {"status", "false"} };
    std::regex pattern(R"(DESCRIBE\s+(\w+)(?:\s+(\w+))?)", std::regex_constants::icase);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = "true";
        result["table"] = match[1];
        if (match[2].matched) result["column"] = match[2];
    }
    return result;
}

/**
 * @brief:decide which parsing function should be used
 * @param:SQL
 * @return:
 */
using ParseFunc = std::map<std::string, std::string>(*)(const std::string&);
std::map<std::string, std::string> parseSQL(const std::string& sql) {
    std::vector<std::pair<std::regex, ParseFunc>> patterns = {
        {std::regex(R"(^CREATE\s)", std::regex_constants::icase), parseCreate},
        {std::regex(R"(^INSERT\s)", std::regex_constants::icase), parseInsert},
        {std::regex(R"(^SELECT\s)", std::regex_constants::icase), parseSelect},
        {std::regex(R"(^USE\s)", std::regex_constants::icase), parseUse},
        {std::regex(R"(^DROP\s)", std::regex_constants::icase), parseDrop},
        {std::regex(R"(^GRANT\s)", std::regex_constants::icase), parseGrant},
        {std::regex(R"(^REVOKE\s)", std::regex_constants::icase), parseRevoke},
        {std::regex(R"(^ALTER\s)", std::regex_constants::icase), parseAlter},
        {std::regex(R"(^SHOW\s)", std::regex_constants::icase), parseShow},
        {std::regex(R"(^UPDATE\s)", std::regex_constants::icase), parseUpdate},
        {std::regex(R"(^DELETE\s)", std::regex_constants::icase), parseDelete},
        {std::regex(R"(^DESCRIBE\s)", std::regex_constants::icase), parseDescribe},
    };
    for (const auto& [pattern, func] : patterns) {
        if (std::regex_search(sql, pattern)) {
            return func(sql);
        }
    }
    return { {"type", "UNKNOWN"}, {"status", "false"} };
}

void Lexer::handleRawSQL(QString rawSql){
    std::string sql=rawSql.toStdString();
    auto result1 = parseSQL(sql);
    std::cout << "Type: " << result1["type"] << ", Status: " << result1["status"] << std::endl;
}

