#include "lexer.h"
#include <iostream>
#include <regex>
#include <map>
#include <vector>
Lexer::Lexer() {}
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

    std::regex createPattern(R"(CREATE\s+(DATABASE|TABLE)\s+(\w+)\s*\(([\s\S]+?)\)\s*;?)", std::regex_constants::icase);
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

    std::cout << result["object_type"] << "\t" << result["object_name"] << std::endl;
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
    std::regex pattern(R"(INSERT\s+INTO\s+(\w+)\s*\(([\w\s,]+)\)\s*VALUES\s*\(([\w\s,]+)\)\s*;?)", std::regex_constants::icase);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = "true";
        result["table"] = match[1];
        result["columns"] = match[2];
        result["values"] = match[3];
    }
    return result;
}

// 解析 SELECT 语句
std::map<std::string, std::string> parseSelect(const std::string& sql) {
    std::map<std::string, std::string> result;
    result["type"] = "SELECT";
    result["status"] = "false";

    std::regex pattern(R"(SELECT\s+([\w\s,*]+)\s+FROM\s+(\w+)(?:\s+WHERE\s+([\w\s=<>']+))?\s*;?)", std::regex_constants::icase);
    std::smatch match;

    if (std::regex_search(sql, match, pattern)) {
        result["status"] = "true";
        result["columns"] = match[1];
        result["table"] = match[2];
        result["where"] = match[3];
    }

    return result;
}

// 根据 SQL 类型调用相应的解析函数
std::map<std::string, std::string> parseSQL(const std::string& sql) {
    std::regex createPattern(R"(^CREATE\s)", std::regex_constants::icase);
    std::regex insertPattern(R"(^INSERT\s)", std::regex_constants::icase);
    std::regex selectPattern(R"(^SELECT\s)", std::regex_constants::icase);

    if (std::regex_search(sql, createPattern)) {
        return parseCreate(sql);
    } else if (std::regex_search(sql, insertPattern)) {
        return parseInsert(sql);
    } else if (std::regex_search(sql, selectPattern)) {
        return parseSelect(sql);
    }

    // 默认返回未知类型
    std::map<std::string, std::string> result;
    result["type"] = "UNKNOWN";
    result["status"] = "false";
    return result;
}

void Lexer::handleRawSQL(QString rawSql){
    std::string sql=rawSql.toStdString();
    //std::string sql = "CREATE TABLE Employees (id INT PRIMARY KEY,name VARCHAR(50) NOT NULL,age INT);";
    auto result1 = parseSQL(sql);
    std::cout << "Type: " << result1["type"] << ", Status: " << result1["status"] << std::endl;
}

