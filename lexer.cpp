#include "lexer.h"
#include <iostream>
#include <regex>
#include <map>
#include <vector>

#define ICASE std::regex_constants::icase

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

    std::regex createPattern(R"(CREATE\s+(DATABASE|TABLE)\s+(\w+)\s*(?:\(([\s\S]*?)\))?\s*;)", ICASE);
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
                std::cout << "columnName" << column["name"] << std::endl;
                std::cout << "columnType" << column["type"] << std::endl;
                std::cout << "columnConstraints" << column["constraints"] << std::endl;
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
    std::regex pattern(R"(DROP\s(DATABASE|TABLE)\s(\w+)\s*(RESTRICT|CASCADE)*;)",ICASE);
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

    // 正则表达式匹配 INSERT INTO 语句
    std::regex pattern(R"(^INSERT\s+INTO\s+(\w+)\s*\(([\w\s,]+)\)\s*VALUES\s*(.+);)", std::regex_constants::icase);
    std::smatch match;

    if (std::regex_search(sql, match, pattern)) {
        result["status"] = "true";
        result["table"] = match[1];  // 表名
        result["columns"] = match[2];  // 列名
        std::string valuesStr = match[3];  // 多组值

        // 提取列名
        std::regex colPattern(R"(\w+)");
        auto colBegin = std::sregex_iterator(result["columns"].begin(), result["columns"].end(), colPattern);
        auto colEnd = std::sregex_iterator();
        std::vector<std::string> columns;

        for (std::sregex_iterator it = colBegin; it != colEnd; ++it) {
            columns.push_back(it->str());
        }

        // 提取多组值
        std::regex groupPattern(R"(\(([^()]+)\))");  // 匹配每组值
        auto groupBegin = std::sregex_iterator(valuesStr.begin(), valuesStr.end(), groupPattern);
        auto groupEnd = std::sregex_iterator();

        for (std::sregex_iterator it = groupBegin; it != groupEnd; ++it) {
            std::string groupValues = it->str(1);  // 获取每组值的内容

            // 提取每组值中的具体值
            std::regex valPattern(R"((?:'[^']*'|[^,]+))");  // 匹配值，支持字符串和普通值
            auto valBegin = std::sregex_iterator(groupValues.begin(), groupValues.end(), valPattern);
            auto valEnd = std::sregex_iterator();
            std::vector<std::string> values;

            for (std::sregex_iterator valIt = valBegin; valIt != valEnd; ++valIt) {
                std::string val = valIt->str();

                // 去掉值前后的空格
                val.erase(0, val.find_first_not_of(' '));
                val.erase(val.find_last_not_of(' ') + 1);

                values.push_back(val);
            }

            // 检查列和值的数量是否一致
            if (columns.size() != values.size()) {
                std::cerr << "Error: Number of columns and values do not match in group: " << groupValues << std::endl;
                continue;
            }

            // 输出每组值的列名和值
            std::cout << "Group Values:" << std::endl;
            for (size_t i = 0; i < columns.size(); ++i) {
                std::cout << "  Column: " << columns[i] << ", Value: " << values[i] << std::endl;
            }
        }
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
    
    std::regex pattern(R"(USE\s+(\w)+;)",ICASE);
    std::smatch match;

    if(std::regex_search(sql,match,pattern)){
        result["status"]="true";
        result["name"] = match[1];
    }
    return result;
}
/**
 * @brief:Parsing SELECT SQL and return relative info.
 * @param: SQL.
 * @return:A map named "result" which has four index:
 *         - "status":whether grammer is correct,
 *         - "type":SELECT(fixed structure),
 *         - "columns": column name,
 *         - "table": table name,
 *         - "where": where condition,
 *         - "group_by": group by condition,
 *         - "having": having condition,
 *         - "order_by": order by condition,
 *         - "limit": limit condition;
 */
std::map<std::string, std::string> parseSelect(const std::string& sql) {
    std::map<std::string, std::string> result = { {"type", "SELECT"}, {"status", "false"} };
    std::regex pattern(R"(SELECT\s+(.*?)\s+FROM\s+(\w+)(?:\s+WHERE\s+(.*?))?(?:\s+GROUP\s+BY\s+(.*?))?(?:\s+HAVING\s+(.*?))?(?:\s+ORDER\s+BY\s+(.*?))?(?:\s+LIMIT\s+(.*?))?)", ICASE);
    std::smatch match;

    if (std::regex_search(sql, match, pattern)) {
        result["status"] = "true";

        // 解析列，按逗号拆分
        std::regex colRegex(R"(\w+(?:\s+AS\s+\w+)?)", ICASE);
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
/**
 * @brief:Parsing GRANT SQL and return relative info.
 * @param: SQL.
 * @return:A map named "result" which has four index:
 *        - "status":whether grammer is correct,
 *        - "type":GRANT(fixed structure),
 *        - "rights": rights granted,
 *        - "object": the rights of which table or database,
 *        - "user": user name,
 * */
std::map<std::string, std::string> parseGrant(const std::string& sql) {
    std::map<std::string, std::string> result = { {"type", "GRANT"}, {"status", "false"} };
    std::regex pattern(R"(GRANT\s+([\w,\s]+)\s+ON\s+(\w+)\s+TO\s+(\w+))", ICASE);
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

/**
 * @brief:Parsing REVOKE SQL and return relative info.
 * @param: SQL.
 * @return:A map named "result" which has four index:
 *        - "status":whether grammer is correct,
 *        - "type":REVOKE(fixed structure),
 *        - "rights": rights revoked,
 *        - "object": the rights of which table or database,
 *        - "user": user name,
 * */
std::map<std::string, std::string> parseRevoke(const std::string& sql) {
    std::map<std::string, std::string> result = { {"type", "REVOKE"}, {"status", "false"} };
    std::regex pattern(R"(REVOKE\s+([\w,\s]+)\s+ON\s+(\w+)\s+FROM\s+(\w+))", ICASE);
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
    std::regex pattern(R"(ALTER\s+TABLE\s+(\w+)\s+(ADD|MODIFY|DROP)\s+(\w+))", ICASE);
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
    std::regex pattern(R"(SHOW\s+(TABLES|DATABASES)(?:\s+FROM\s+(\w+))?)", ICASE);
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
    std::regex pattern(R"(UPDATE\s+(\w+)\s+SET\s+(\w+)\s*=\s*(\w+)(?:\s+WHERE\s+(.+))?)", ICASE);
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
    std::regex pattern(R"(DELETE\s+FROM\s+(\w+)(?:\s+WHERE\s+(.+))?)", ICASE);
    std::smatch match;
    if (std::regex_search(sql, match, pattern)) {
        result["status"] = "true";
        result["table"] = match[1];
        if (match[2].matched) result["condition"] = match[2];
    }
    return result;
}

/**
 * @brief:Parsing DESCRIBE SQL and return relative info.
 * @param: SQL.
 * @return:A map named "result" which has four index:
 *       - "status":whether grammer is correct,
 *       - "type":DESCRIBE(fixed structure),
 *       - "table": table name,
 *       - "column": column name;
 */
std::map<std::string, std::string> parseDescribe(const std::string& sql) {
    std::map<std::string, std::string> result = { {"type", "DESCRIBE"}, {"status", "false"} };
    std::regex pattern(R"(DESCRIBE\s+(\w+)(?:\s+(\w+))?)", ICASE);
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
 * @return:the result of parsing function
 */
using ParseFunc = std::map<std::string, std::string>(*)(const std::string&);
std::map<std::string, std::string> parseSQL(const std::string& sql) {
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
        {std::regex(R"(^DESCRIBE\s)", ICASE), parseDescribe},
    };
    for (const auto& [pattern, func] : patterns) {
        if (std::regex_search(sql, pattern)) {
            return func(sql);
        }
    }
    return { {"type", "UNKNOWN"}, {"status", "false"} };
}
/**
 * @brief:handleRawSQL is the main function of Lexer class.
 * It will call parseSQL to parse all the SQL from MainWindow.
 */
void Lexer::handleRawSQL(QString rawSql){
    std::string sql=rawSql.toStdString();
    auto result1 = parseSQL(sql);
    std::cout << "Type: " << result1["type"] << ", Status: " << result1["status"] << std::endl;
}

