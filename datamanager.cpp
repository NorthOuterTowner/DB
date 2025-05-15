#include "datamanager.h"
#include "affair.h"
#include "logger.h"
#include "session.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QString>
#include <QStringList>
#include <QIODevice>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <unordered_set>
#include <cctype>
#include "lexer.h"
namespace fs=std::filesystem;

datamanager::datamanager(dbManager* dbMgr) : dbMgr(dbMgr), affair("undo.txt",nullptr) {
    // 构造函数初始化 affair
}

std::string datamanager::getCurrentDatabase() const{
    if(dbMgr){
        return dbMgr->getCurrentDatabase();//通过dbManager获取当前数据库
    }
    return "";//如果dbMgr为null，返回空字符串
}


// 在 datamanager 类中添加备份数据文件的函数
void datamanager::backupDataFile(const std::string& dbName, const std::string& tableName) {
    std::string dataFilePath = buildFilePath(dbName, tableName);
    std::string backupDataFilePath = "../../res/backup/" + tableName + "_data_backup.txt";
    QFile dataFile(QString::fromStdString(dataFilePath));
    QFile backupDataFile(QString::fromStdString(backupDataFilePath));

    if (dataFile.open(QIODevice::ReadOnly | QIODevice::Text) && backupDataFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream in(&dataFile);
        QTextStream out(&backupDataFile);
        out << in.readAll();
        dataFile.close();
        backupDataFile.close();
    }
}

// 构建文件路径（与 tableManage 保持一致：../../res/表名.data.txt）
std::string datamanager::buildFilePath(const std::string& dbName, const std::string& tableName) {
    return "../../res/" + tableName + ".data.txt";

}

bool datamanager::tryStringtoInt(const std::string& s, int& out) {
    if (s.empty()) return false;
    try {
        size_t pos = 0;
        out = std::stoi(s, &pos);
        // 检查是否整个字符串都被转换了 并且没有剩余非数字字符
        return pos == s.size();
    } catch (const std::invalid_argument& ia) {
        // 无效参数 不是一个有效的数字格式
        return false;
    } catch (const std::out_of_range& oor) {
        // 超出int范围
        return false;
    }
}

bool datamanager::tryStringtoDouble(const std::string& s, double& out) {
    if (s.empty()) return false;
    try {
        size_t pos = 0;
        out = std::stod(s, &pos);
        // 检查是否整个字符串都被替换了
        return pos == s.size();
    } catch (const std::invalid_argument& ia) {
        return false;
    } catch (const std::out_of_range& oor) {
        return false;
    }
}

bool datamanager::tryStringtoBool(const std::string& s, bool& out) {
    // 简化处理：接收 true、false、1、0
    std::string lower_s;
    lower_s.resize(s.size());
    // 将转换结果存储到 lower_s 中
    std::transform(s.begin(), s.end(), lower_s.begin(), ::tolower);

    if (lower_s == "true" || lower_s == "1" || lower_s == "TRUE") {
        out = true;
        return true;
    } else if (lower_s == "false" || lower_s == "0" || lower_s == "FALSE") {
        out = false;
        return true;
    }
    return false;
}

std::vector<std::string> datamanager::splitString(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    while (std::getline(tokenStream, token, delimiter)) {
        // 过滤空字段并去除首尾空格
        if (!token.empty()) {
            token.erase(0, token.find_first_not_of(" \t\n\r"));
            token.erase(token.find_last_not_of(" \t\n\r") + 1);
            tokens.push_back(token);
        }
    }
    return tokens;
}

// 辅助函数：根据列名查找列的索引
int datamanager::findColumnIndex(const std::vector<fieldManage::FieldInfo>& columnsInfo, const std::string& columnName) {
    for (size_t i = 0; i < columnsInfo.size(); ++i) {
        if (columnsInfo[i].fieldName == columnName) {
            return static_cast<int>(i);
        }
    }
    return -1; // 未找到
}

// 辅助函数：将字符串值根据指定的列类型进行转换以便比较或验证
// 处理从解析器中可能带有的单引号字符串（例如 'Alice' -> Alice）
// 此函数用于将数据文件中的值和 WHERE/SET 子句中的值转换为列的实际类型。


// --- 求值 WHERE 子句的 AST 函数 ---




bool datamanager::convertValueForComparison(
    const std::string& valueStr,
    const std::string& columnType,
    int& intVal, double& doubleVal, bool& boolVal, std::string& stringVal
    ) {
    std::string upperType = columnType;
    std::transform(upperType.begin(), upperType.end(), upperType.begin(), ::toupper);

    if (upperType == "INT") {
        try {
            intVal = std::stoi(valueStr);
            return true;
        } catch (...) {
            return false;
        }
    } else if (upperType == "DOUBLE" || upperType == "FLOAT") {
        try {
            doubleVal = std::stod(valueStr);
            return true;
        } catch (...) {
            return false;
        }
    } else if (upperType == "BOOL" || upperType == "BOOLEAN") {
        if (valueStr == "true" || valueStr == "TRUE" || valueStr == "1") {
            boolVal = true;
            return true;
        } else if (valueStr == "false" || valueStr == "FALSE" || valueStr == "0") {
            boolVal = false;
            return true;
        }
        return false;
    } else {
        stringVal = valueStr;
        return true;
    }
//<<<<<<< HEAD
}




//=======

//     if (typeUpper == "INT") {
//         return tryStringtoInt(cleanedValueStr, intVal); // 尝试转换为 INT
//     } else if (typeUpper == "DOUBLE" || typeUpper == "FLOAT") {
//         return tryStringtoDouble(cleanedValueStr, doubleVal); // 尝试转换为 DOUBLE/FLOAT
//     } else if (typeUpper == "BOOL" || typeUpper == "BOOLEAN") {
//         return tryStringtoBool(cleanedValueStr, boolVal); // 尝试转换为 BOOL
//     } else if (typeUpper == "VARCHAR" || typeUpper == "TEXT") {
//         stringVal = cleanedValueStr; // 对于字符串类型，直接存储处理过的字符串
//         return true; // 字符串本身转换为字符串总是成功的
//     }
//     // 如果有其他数据类型（如 DATE, TIME 等），在这里添加转换逻辑

//     // 如果遇到未知类型，返回 false 表示转换失败
//     return false;
// }

// --- 求值 WHERE 子句的 AST 函数 ---
//>>>>>>> 9e3794f9e4ed8e4a413a17e5976ebb9162f7bbed
// 递归函数，用于判断一行数据是否符合 WHERE 子句的条件树
bool datamanager::evaluateWhereClauseTree(
    const std::shared_ptr<Node>& whereTree,
    const std::vector<std::string>& rowValues,
    const std::vector<fieldManage::FieldInfo>& columnsInfo
    ) {
    if (!whereTree) {
        return true;
    }

    if (std::holds_alternative<Condition>(*whereTree)) {
        const auto& cond = std::get<Condition>(*whereTree);
        int colIndex = findColumnIndex(columnsInfo, cond.column);
        if (colIndex == -1) {
            std::cerr << "Error evaluating WHERE tree: Unknown column '" << cond.column << "'" << std::endl;
            return false;
        }

        if (colIndex >= rowValues.size()) {
            std::cerr << "Error evaluating WHERE tree: Column index " << colIndex << " out of bounds for row with " << rowValues.size() << " values." << std::endl;
            return false;
        }

        const std::string& columnType = columnsInfo[colIndex].fieldType;
        const std::string& rowValueStr = rowValues[colIndex];
        const std::string& clauseValueStr = cond.value;

        int rowInt, clauseInt;
        double rowDouble, clauseDouble;
        bool rowBool, clauseBool;
        std::string rowString, clauseString;

        bool rowConversionOk = convertValueForComparison(rowValueStr, columnType, rowInt, rowDouble, rowBool, rowString);
        bool clauseConversionOk = convertValueForComparison(clauseValueStr, columnType, clauseInt, clauseDouble, clauseBool, clauseString);

        if (!rowConversionOk || !clauseConversionOk) {
            std::cerr << "Warning: Type conversion failed during WHERE tree evaluation for column '" << cond.column
                      << "' (type: " << columnType << ") comparing row value '" << rowValueStr
                      << "' with clause value '" << clauseValueStr << "'. Condition is false." << std::endl;
            return false;
        }

        std::string opUpper = cond.op;
        std::transform(opUpper.begin(), opUpper.end(), opUpper.begin(), ::toupper);

        std::string upperType = columnType;
        std::transform(upperType.begin(), upperType.end(), upperType.begin(), ::toupper);

        std::cout << "[DEBUG] WHERE Clause Evaluation for " << upperType << ": Row Int: " << rowInt << ", Clause Int: " << clauseInt << ", Operator: " << opUpper << std::endl;

        if (upperType == "INT") {
            if (opUpper == "=" || opUpper == "==") return rowInt == clauseInt;
            if (opUpper == "!=") return rowInt != clauseInt;
            if (opUpper == ">") return rowInt > clauseInt;
            if (opUpper == "<") return rowInt < clauseInt;
            if (opUpper == ">=") return rowInt >= clauseInt;
            if (opUpper == "<=") return rowInt <= clauseInt;
        } else if (upperType == "DOUBLE" || upperType == "FLOAT") {
            if (opUpper == "=" || opUpper == "==") return rowDouble == clauseDouble;
            if (opUpper == "!=") return rowDouble != clauseDouble;
            if (opUpper == ">") return rowDouble > clauseDouble;
            if (opUpper == "<") return rowDouble < clauseDouble;
            if (opUpper == ">=") return rowDouble >= clauseDouble;
            if (opUpper == "<=") return rowDouble <= clauseDouble;
        } else if (upperType == "BOOL" || upperType == "BOOLEAN") {
            if (opUpper == "=" || opUpper == "==") return rowBool == clauseBool;
            if (opUpper == "!=") return rowBool != clauseBool;
            return false;
        } else {
            if (opUpper == "=" || opUpper == "==") return rowString == clauseString;
            if (opUpper == "!=") return rowString != clauseString;
            if (opUpper == ">") return rowString > clauseString;
            if (opUpper == "<") return rowString < clauseString;
            if (opUpper == ">=") return rowString >= clauseString;
            if (opUpper == "<=") return rowString <= clauseString;
            return false;
        }
    } else if (std::holds_alternative<LogicalOp>(*whereTree)) {
        const auto& logOp = std::get<LogicalOp>(*whereTree);

        bool leftResult = evaluateWhereClauseTree(logOp.left, rowValues, columnsInfo);

        std::string opUpper = logOp.op;
        std::transform(opUpper.begin(), opUpper.end(), opUpper.begin(), ::toupper);

        if (opUpper == "AND") {
            if (!leftResult) return false;
        } else if (opUpper == "OR") {
            if (leftResult) return true;
        } else {
            std::cerr << "Internal Error: Unknown logical operator '" << logOp.op << "' in WHERE tree." << std::endl;
            return false;
        }

        bool rightResult = evaluateWhereClauseTree(logOp.right, rowValues, columnsInfo);

        if (opUpper == "AND") {
            return leftResult && rightResult;
        } else if (opUpper == "OR") {
            return leftResult || rightResult;
        }

        return false;
    }

    std::cerr << "Internal Error: Unexpected node type in WHERE tree." << std::endl;
    return false;
}

// 辅助函数：解析 SET 子句 ("col1=val1, col2=val2") 为列索引和值的映射
// 返回一个 map，键是列的索引，值是要更新的字符串
std::map<int, std::string> datamanager::parseSetClause(
    const std::string& setClause,
    const std::vector<fieldManage::FieldInfo>& columnsInfo
    ) {
    std::map<int, std::string> updates; // 存储更新的列索引和新值
    if (setClause.empty()) {
        return updates; // 空 SET 子句返回空 map
    }

    std::vector<std::string> parts = splitString(setClause, ','); // 按逗号分割每个更新对

    for (const auto& part : parts) {
        // 查找 '=' 的位置
        size_t eqPos = part.find('=');
        if (eqPos == std::string::npos) {
            std::cerr << "Error parsing SET clause: Invalid format in part '" << part << "'. Expected 'column=value'." << std::endl;
            // 如果格式错误，返回空 map 表示失败
            return {};
        }

        // 提取列名和值字符串
        std::string columnName = part.substr(0, eqPos);
        std::string valueStr = part.substr(eqPos + 1);

        // 去除列名和值两端的空白字符
        columnName.erase(0, columnName.find_first_not_of(" \t"));
        columnName.erase(columnName.find_last_not_of(" \t") + 1);
        valueStr.erase(0, valueStr.find_first_not_of(" \t"));
        valueStr.erase(valueStr.find_last_not_of(" \t") + 1);

        // 查找列名对应的索引
        int colIndex = findColumnIndex(columnsInfo, columnName);
        if (colIndex == -1) {
            std::cerr << "Error parsing SET clause: Unknown column '" << columnName << "'" << std::endl;
            // 列名不存在，返回空 map 表示失败
            return {};
        }

        // 验证新值是否符合列的 NOT NULL 约束和数据类型
        const fieldManage::FieldInfo& columnInfo = columnsInfo[colIndex];
        bool isNotNull = (columnInfo.constraints.find("NOT NULL") != std::string::npos);

        if (isNotNull && valueStr.empty()) {
            std::cerr << "Validation Error (SET clause): Column '" << columnName << "' cannot be NULL (empty string)." << std::endl;
            // 违反 NOT NULL 约束，返回空 map 表示失败
            return {};
        }

        // 如果新值不为空，检查它是否可以转换为列的类型
        // 如果新值为空且通过了 NOT NULL 检查，则是合法的（例如设置为允许 NULL 的列的空值）
        if (!valueStr.empty()) {
            int intVal; double doubleVal; bool boolVal; std::string stringVal; // 占位符变量
            // 使用 convertValueForComparison 进行类型验证
            if (!convertValueForComparison(valueStr, columnInfo.fieldType, intVal, doubleVal, boolVal, stringVal)) {
                std::cerr << "Validation Error (SET clause): Value '" << valueStr
                          << "' is not a valid format for column '" << columnName
                          << "' of type '" << columnInfo.fieldType << "'." << std::endl;
                // 值格式与列类型不匹配，返回空 map 表示失败
                return {};
            }
        }

        updates[colIndex] = valueStr; // 将列索引和新值存储到 map 中
    }

    return updates; // 返回包含所有有效更新的 map
}

// 辅助函数：将一个字符串向量（一行数据）用逗号连接成一个字符串
std::string datamanager::joinRowValues(const std::vector<std::string>& rowValues) {
    std::ostringstream oss;
    for (size_t i = 0; i < rowValues.size(); ++i) {
        if (i > 0) {
            oss << ","; // 在除第一个值外的所有值前面添加逗号
        }
        oss << rowValues[i];
    }
    return oss.str();
}

// 实现 updateTableRecordCount 方法
void datamanager::updateTableRecordCount(const std::string& dbName, const std::string& tableName, int count) {
    tableMgr.updateTableRecordCount(dbName, tableName, count);
}

void datamanager::updateTableLastModifiedDate(const std::string& dbName, const std::string& tableName) {
    std::string tableDescFile = "../../res/" + dbName + ".tb.txt";
    QFile tbFile(QString::fromStdString(tableDescFile));

    if (!tbFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "Failed to open table description file: " << tableDescFile << std::endl;
        return;
    }

    QStringList allLines;
    QTextStream in(&tbFile);
    while (!in.atEnd()) {
        allLines.append(in.readLine());
    }
    tbFile.close();

    bool found = false;
    for (int i = 0; i < allLines.size(); ++i) {
        std::vector<std::string> parts;
        QString line = allLines[i];
        QStringList lineParts = line.split(" ");
        for (const auto& part : lineParts) {
            parts.push_back(part.toStdString());
        }

        if (parts.size() > 1 && parts[0] == tableName) {
            found = true;
            // 更新修改时间为当前时间
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%H:%M:%S");
            std::string modificationTime = oss.str();
            parts[3] = modificationTime; // 假设修改时间在第 4 列

            QString newLine;
            for (const auto& part : parts) {
                newLine += QString::fromStdString(part) + " ";
            }
            allLines[i] = newLine.trimmed();
            break;
        }
    }

    if (!found) {
        std::cerr << "Table " << tableName << " not found in database " << dbName << std::endl;
        return;
    }

    if (!tbFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::cerr << "Failed to open table description file for writing: " << tableDescFile << std::endl;
        return;
    }

    QTextStream out(&tbFile);
    for (const auto& line : allLines) {
        out << line << "\n";
    }
    tbFile.close();
}

bool datamanager::insertData(const std::string& dbName, const std::string& tableName, const std::vector<std::string>& values) {
    // 记录事务操作
    QString sql = QString("INSERT INTO %1 VALUES (").arg(QString::fromStdString(tableName));
    for (size_t i = 0; i < values.size(); ++i) {
        sql += QString("'%1'").arg(QString::fromStdString(values[i]));
        if (i < values.size() - 1) {
            sql += ", ";
        }
    }
    sql += ");";
    affair.writeToUndo(sql);

    // 获取表信息
    tableManage::TableInfo tableInfo = tableMgr.getTableInfo(dbName, tableName);
    if (tableInfo.table_name.empty()) {
        std::cerr << "Table " << tableName << " does not exist in database " << dbName << std::endl;
        return false;
    }

    // 获取表的详细列信息
    std::vector<fieldManage::FieldInfo> columnsInfo = fieldMgr.getFieldsInfo(dbName, tableName);

    // 验证插入数据
    if (!validateInsertData(tableInfo, columnsInfo, values)) {
        std::cerr << "Invalid data for table " << tableName << std::endl;
        return false;
    }

    // 构建数据文件路径
//<<<<<<< HEAD
    //std::string dataFilePath = "../../res/" + tableName + ".data.txt";

     std::string dataFilePath = buildFilePath(dbName,tableName);

    //std::string dataFilePath = buildFilePath(dbName, tableName);


     // 如果数据文件不存在，先创建一个空文件
     std::ifstream checkFile(dataFilePath);
     if (!checkFile.good()) {
         std::ofstream createFile(dataFilePath); // 创建空文件
         if (!createFile.is_open()) {
             std::cerr << "Failed to create data file: " << dataFilePath << std::endl;
             return false;
         }
         // 写入表头
         for (size_t i = 0; i < columnsInfo.size(); ++i) {
             if (i > 0) {
                 createFile << ",";
             }
             createFile << columnsInfo[i].fieldName;
         }
         createFile << std::endl;
         createFile.close();
         std::cout << "[DEBUG] Data file created: " << dataFilePath << std::endl;
     }

   // std::string dataFilePath = buildFilePath(dbName, tableName);
//>>>>>>> 9e3794f9e4ed8e4a413a17e5976ebb9162f7bbed

    // 如果数据文件不存在，先创建一个空文件



    // 打开数据文件以追加模式写入
    std::ofstream dataFile(dataFilePath, std::ios::app);
    if (!dataFile.is_open()) {
        std::cerr << "Failed to open data file: " << dataFilePath << std::endl;
        return false;
    }

    // 写入数据
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            dataFile << ",";
        }
        dataFile << values[i];
    }
    dataFile << std::endl;

    // 关闭数据文件
    dataFile.close();

    // 更新表的记录数
    // updateTableRecordCount(dbName, tableName, 1);

    // 更新表的最后修改时间
    // updateTableLastModifiedDate(dbName, tableName);

    // 记录插入操作的日志
    Logger logger("../../res/system_logs.txt");
    logger.log(Session::getCurrentUserId(), "INSERT", "DATA", "Inserted values into " + tableName + " in " + dbName); // 记录日志

    std::cout << "数据插入成功，路径：" << dataFilePath << std::endl;
    return true;
}

bool datamanager::validateInsertData(const tableManage::TableInfo& tableInfo, const std::vector<fieldManage::FieldInfo>& columnsInfo, const std::vector<std::string>& values) {
    // 检查值的数量是否与列数一致
    if (values.size() != columnsInfo.size()) {
        std::cerr << "Validation Error: Number of values (" << values.size()
        << ") does not match the number of columns (" << columnsInfo.size()
        << ") in table '" << tableInfo.table_name << "'." << std::endl;
        return false;
    }

    // 逐个值进行检查（NOT NULL 和数据类型）
    for (size_t i = 0; i < values.size(); ++i) {
        const fieldManage::FieldInfo& column = columnsInfo[i];
        const std::string& value = values[i];

        // 检查 NOT NULL 约束
        bool isNotNull = (column.constraints.find("NOT NULL") != std::string::npos);
        if (isNotNull && value.empty()) {
            std::cerr << "Validation Error: Column '" << column.fieldName
                      << "' (index " << i << ") cannot be NULL (empty string)." << std::endl;
            return false;
        }

        // 如果值不为空，则进行数据类型检查
        if (!value.empty()) {
            // 将列类型转换为大写以便比较
            std::string columnTypeUpper = column.fieldType;
            std::transform(columnTypeUpper.begin(), columnTypeUpper.end(), columnTypeUpper.begin(), ::toupper);

            if (columnTypeUpper == "INT") {
                int int_val;
                if (!tryStringtoInt(value, int_val)) {
                    std::cerr << "Validation Error: Value '" << value
                              << "' for column '" << column.fieldName << "' (index " << i
                              << ") is not a valid INT." << std::endl;
                    return false;
                }
            } else if (columnTypeUpper == "DOUBLE" || columnTypeUpper == "FLOAT") {
                double double_val;
                if (!tryStringtoDouble(value, double_val)) {
                    std::cerr << "Validation Error: Value '" << value
                              << "' for column '" << column.fieldName << "' (index " << i
                              << ") is not a valid DOUBLE/FLOAT." << std::endl;
                    return false;
                }
            } else if (columnTypeUpper == "BOOL" || columnTypeUpper == "BOOLEAN") {
                bool bool_val;
                if (!tryStringtoBool(value, bool_val)) {
                    std::cerr << "Validation Error: Value '" << value
                              << "' for column '" << column.fieldName << "' (index " << i
                              << ") is not a valid BOOL (expected 'true', 'false', '0', or '1')." << std::endl;
                    return false;
                }
            }
            // 对于 VARCHAR/TEXT 类型，如果值不为空且 NOT NULL 约束已通过，则认为有效
            // 如果需要限制 VARCHAR 长度，可以在这里添加检查
            else if (columnTypeUpper == "VARCHAR" || columnTypeUpper == "TEXT") {
                // 基本验证已通过（非空且不是 NOT NULL）
            }
            // 添加更多数据类型的检查，例如 DATE, TIME 等...
            else {
                // 如果遇到未知的数据类型，可以视为错误或跳过验证（取决于你的需求）
                std::cerr << "Validation Warning: Unknown data type '" << column.fieldType
                          << "' for column '" << column.fieldName << "' (index " << i
                          << "). Skipping type validation for this column." << std::endl;
                // return false; // 如果未知类型视为错误
            }
        }
    }

    // 如果所有检查都通过，则数据有效
    return true;
}




bool datamanager::deleteData(const std::string& dbName,const std::string& tableName, const std::string& primaryKeyValue
    ) {

    // 判断是否是整表删除
    if (primaryKeyValue == "ALL") {
        std::string dataFilePath = buildFilePath(dbName, tableName);

        // 删除整个数据文件
        if (std::remove(dataFilePath.c_str()) == 0) {
            std::cout << "[INFO] Entire table " << tableName << " deleted successfully." << std::endl;

            // 创建一个新的空文件，保证表结构文件和数据文件一致
            std::ofstream newFile(dataFilePath);
            if (!newFile.is_open()) {
                std::cerr << "[ERROR] Failed to recreate empty data file for table: " << tableName << std::endl;
                return false;
            }
            newFile.close();

            updateTableRecordCount(dbName, tableName, 0);  // 重置记录数
            updateTableLastModifiedDate(dbName, tableName);
            return true;
        } else {
            std::cerr << "[ERROR] Failed to delete entire table: " << tableName << std::endl;
            return false;
        }
    }



    // 获取表结构
    tableManage::TableInfo tableInfo = tableMgr.getTableInfo(dbName, tableName);
    if (tableInfo.table_name.empty()) {
        std::cerr << "Error: Table does not exist.\n";
        return false;
    }

    std::vector<fieldManage::FieldInfo> columnsInfo = fieldMgr.getFieldsInfo(dbName, tableName);
    int primaryKeyIndex = -1;
    for (int i = 0; i < columnsInfo.size(); ++i) {
        std::string constraintLower = columnsInfo[i].constraints;
        std::transform(constraintLower.begin(), constraintLower.end(), constraintLower.begin(), ::tolower);
        if (constraintLower.find("primary key") != std::string::npos) {
            primaryKeyIndex = i;
            break;
        }
    }
    if (primaryKeyIndex == -1) {
        std::cerr << "Error: No primary key found.\n";
        return false;
    }

    std::string dataFilePath = buildFilePath(dbName, tableName);
    std::string tempFilePath = dataFilePath + ".tmp";

    std::ifstream inFile(dataFilePath);
    std::ofstream outFile(tempFilePath);
    if (!inFile.is_open() || !outFile.is_open()) {
        std::cerr << "Error opening file.\n";
        return false;
    }

    std::string line;
    bool deleted = false;
    while (std::getline(inFile, line)) {
        std::vector<std::string> row = splitString(line, ',');
        if (row.size() != columnsInfo.size()) {
            outFile << line << "\n";
            continue;
        }

        if (row[primaryKeyIndex] == primaryKeyValue) {
            deleted = true;
            continue; // 跳过该行
        }

        outFile << joinRowValues(row) << "\n";
    }

    inFile.close();
    outFile.close();

    if (deleted) {
        std::remove(dataFilePath.c_str());
        std::rename(tempFilePath.c_str(), dataFilePath.c_str());
        updateTableRecordCount(dbName, tableName, -1);
        updateTableLastModifiedDate(dbName, tableName);
        return true;
    } else {
        std::remove(tempFilePath.c_str());
        std::cerr << "No matching row found.\n";
        return false;
    }

}



int findColumnIndexByName(const std::vector<fieldManage::FieldInfo>& columnsInfo, const std::string& columnName) {
    for (size_t i = 0; i < columnsInfo.size(); ++i) {
        if (columnsInfo[i].fieldName == columnName) {
            return static_cast<int>(i); // 找到列名匹配的索引
        }
    }
    return -1; // 如果没有找到，返回-1表示错误
}






std::vector<std::vector<std::string>> datamanager::readTableData(const std::string& dbName, const std::string& tableName) {
    std::vector<std::vector<std::string>> tableData;

    // 构建表文件路径
    std::string filePath = buildFilePath(dbName, tableName);

    // 打开表文件
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open table file: " << filePath << std::endl;
        return tableData;
    }

    // 获取表的字段信息
    std::vector<fieldManage::FieldInfo> fields = fieldMgr.getFieldsInfo(dbName, tableName);

    std::string line;
    // 跳过表头行
    if (std::getline(file, line)) {
        // 读取数据行
        while (std::getline(file, line)) {
            // 分割每行数据
            std::vector<std::string> values = splitString(line, ',');

            // 验证数据行长度是否与表结构匹配
            if (values.size() == fields.size()) {
                tableData.push_back(values);
            } else {
                std::cerr << "[WARNING] Invalid data row in table " << tableName << ": " << line << std::endl;
            }
        }
    }

    file.close();
    return tableData;
}

std::vector<std::string> datamanager::getTableColumns(const std::string& dbName, const std::string& tableName) {
    std::vector<std::string> columnNames;

    // 使用 fieldManage 获取表的字段信息
    std::vector<fieldManage::FieldInfo> fields = fieldMgr.getFieldsInfo(dbName, tableName);

    // 从字段信息中提取列名
    for (const auto& field : fields) {
        columnNames.push_back(field.fieldName);
    }

    return columnNames;
}








std::vector<std::string> datamanager::calculateGlobalAggregates(
    const std::string& tableName,
    const std::vector<std::string>& aggregateFunctions,
    const std::shared_ptr<Node>& whereTree
    ) {
    std::vector<std::string> result;
    std::string dbName = getCurrentDatabase();
    std::vector<std::vector<std::string>> tableData = readTableData(dbName, tableName);
    std::vector<fieldManage::FieldInfo> columnsInfo = fieldMgr.getFieldsInfo(dbName, tableName);

    std::vector<std::pair<std::string, std::string>> parsedAggregateFunctions;
    for (const auto& func : aggregateFunctions) {
        size_t openPos = func.find('(');
        size_t closePos = func.find(')');
        if (openPos != std::string::npos && closePos != std::string::npos && openPos < closePos) {
            std::string funcName = utils::toUpper(utils::strip(func.substr(0, openPos)));
            std::string param = utils::strip(func.substr(openPos + 1, closePos - openPos - 1));
            parsedAggregateFunctions.emplace_back(funcName, param);
        }
    }

    int count = 0;
    for (const auto& row : tableData) {
        if (evaluateWhereClauseTree(whereTree, row, columnsInfo)) {
            count++;
        }
    }

    for (const auto& [funcName, param] : parsedAggregateFunctions) {
        if (funcName == "COUNT") {
            result.push_back(std::to_string(count));
        } else {
            result.push_back("NULL");
        }
    }

    return result;
}

// 执行 SELECT 查询
// std::vector<std::vector<std::string>> datamanager::selecData(
//     const std::string& dbName,
//     const std::string& tableName,
//     const std::vector<std::string>& columnsToSelect,
//     const std::shared_ptr<Node>& whereTree,
//     const std::vector<std::string>& aggregateFunctions,
//     const std::vector<std::string>& groupByColumns
//     ) {
//     std::vector<std::vector<std::string>> results;

//     // 检查输入参数
//     std::cout << "[DEBUG] Executor - Input parameters:" << std::endl;
//     std::cout << "[DEBUG]   dbName: " << dbName << std::endl;
//     std::cout << "[DEBUG]   tableName: " << tableName << std::endl;
//     std::cout << "[DEBUG]   columnsToSelect: ";
//     for (const auto& col : columnsToSelect) std::cout << col << " ";
//     std::cout << std::endl;
//     std::cout << "[DEBUG]   aggregateFunctions: ";
//     for (const auto& func : aggregateFunctions) std::cout << func << " ";
//     std::cout << std::endl;
//     std::cout << "[DEBUG]   groupByColumns: ";
//     for (const auto& col : groupByColumns) std::cout << col << " ";
//     std::cout << std::endl;




//     //1. 验证表是否存在
//         tableManage::TableInfo tableInfo = tableMgr.getTableInfo(dbName, tableName);
//         if (tableInfo.table_name.empty()) {
//             std::cerr << "Error selecting data: Table '" << tableName << "' does not exist in database '" << dbName << "'." << std::endl;
//             return results; // 表不存在，返回空结果
//         }

//         // 2. 获取表的列信息
//         std::vector<fieldManage::FieldInfo> columnsInfo = fieldMgr.getFieldsInfo(dbName,tableName);
//         if (columnsInfo.empty()) {
//             std::cerr << "Error selecting data: Could not retrieve column information for table '" << tableName << "'." << std::endl;
//             // 即使是空表也应该有列信息，这是一个错误
//             return results; // 获取列信息失败，返回空结果
//         }

//         // // 3. 确定要选择的列以及它们在数据行中的索引
//         // std::vector<int> selectColumnIndices; // 存储要选择的列的索引
//         // if (columnsToSelect.empty()) {
//         //     // 如果未指定列，则选择所有列
//         //     for (size_t i = 0; i < columnsInfo.size(); ++i) {
//         //         selectColumnIndices.push_back(static_cast<int>(i));
//         //     }
//         // } else {
//         //     // 如果指定了列，则查找这些列的索引
//         //     for (const auto& colName : columnsToSelect) {
//         //         int index = findColumnIndex(columnsInfo, colName);
//         //         if (index == -1) {
//         //             std::cerr << "Error selecting data: Unknown column '" << colName << "' specified in SELECT list for table '" << tableName << "'." << std::endl;
//         //             return results; // SELECT 列表中包含未知列，返回空结果并报错
//         //         }
//         //         selectColumnIndices.push_back(index);
//         //     }
//         // }

//         std::vector<std::string> actualColumnsToSelect;
//         if (columnsToSelect.size() == 1 && columnsToSelect[0] == "ALL_COLUMNS") {
//             // 将 "ALL_COLUMNS" 扩展为实际的列名列表
//             for (const auto& colInfo : columnsInfo) {
//                 actualColumnsToSelect.push_back(colInfo.fieldName);
//             }
//             std::cout << "[DEBUG] Expanded 'ALL_COLUMNS' to: ";
//             for (const auto& col : actualColumnsToSelect) std::cout << col << " ";
//             std::cout << std::endl;
//         } else {
//             actualColumnsToSelect = columnsToSelect;
//         }


//         // 确定要选择的列以及它们在数据行中的索引
//         std::vector<int> selectColumnIndices;
//         for (const auto& colName : actualColumnsToSelect) {
//             int index = findColumnIndex(columnsInfo, colName);
//             if (index == -1) {
//                 std::cerr << "Error selecting data: Unknown column '" << colName << "' specified in SELECT list for table '" << tableName << "'." << std::endl;
//                 return results;
//             }
//             selectColumnIndices.push_back(index);
//         }


//     // 解析聚合函数
//     std::vector<std::pair<std::string, std::string>> parsedAggregateFunctions;
//     for (const auto& func : aggregateFunctions) {
//         size_t openPos = func.find('(');
//         size_t closePos = func.find(')');
//         if (openPos != std::string::npos && closePos != std::string::npos && openPos < closePos) {
//             std::string funcName = utils::toUpper(utils::strip(func.substr(0, openPos)));
//             std::string param = utils::strip(func.substr(openPos + 1, closePos - openPos - 1));
//             parsedAggregateFunctions.emplace_back(funcName, param);
//         }
//     }

//     // 无聚合函数的简单查询
//     if (aggregateFunctions.empty()) {
//         std::vector<int> selectIndices;
//         if (columnsToSelect.empty()) {
//             // 选择所有列
//             for (size_t i = 0; i < columnsInfo.size(); i++) {
//                 selectIndices.push_back(i);
//             }
//         } else {
//             // 选择指定列
//             for (const auto& col : columnsToSelect) {
//                 int idx = findColumnIndex(columnsInfo, col);
//                 if (idx == -1) {
//                     std::cerr << "Error: Unknown column '" << col << "' in table '" << tableName << "'." << std::endl;
//                     return results;
//                 }
//                 selectIndices.push_back(idx);
//             }
// //<<<<<<< HEAD
//         }

//         // 读取数据文件
//         std::string dataFilePath = "../../res/" + tableName + ".data.txt";
//         std::ifstream dataFile(dataFilePath);
//         if (!dataFile.is_open()) {
//             std::cerr << "Error: Could not open data file for table '" << tableName << "'." << std::endl;
//             return results;
//         }

//         std::string line;
//         int rowCount = 0;
//         while (std::getline(dataFile, line)) {
//             rowCount++;
//             std::vector<std::string> rowValues = splitString(line, ',');

//             // 验证行格式
//             if (rowValues.size() != columnsInfo.size()) {
//                 std::cerr << "Warning: Skipping malformed row " << rowCount << " (expected "
//                           << columnsInfo.size() << " columns, got " << rowValues.size() << ")" << std::endl;
//                 continue;
//             }

//             // 应用 WHERE 过滤
//             if (whereTree && !evaluateWhereClauseTree(whereTree, rowValues, columnsInfo)) {
//                 continue;
//             }

//             // 收集结果行
//             std::vector<std::string> selectedRow;
//             for (int idx : selectIndices) {
//                 selectedRow.push_back(rowValues[idx]);
//             }
//             results.push_back(selectedRow);
//         }

//         dataFile.close();
//         return results;
//     }

//     // 处理聚合查询
//     if (!groupByColumns.empty()) {
//         // 带 GROUP BY 的聚合查询
//         std::unordered_map<std::string, std::vector<std::vector<std::string>>> groupedRows;

//         // 读取数据文件并分组
//         std::string dataFilePath = "../../res/" + tableName + ".data.txt";
//         std::ifstream dataFile(dataFilePath);
//         if (!dataFile.is_open()) {
//             std::cerr << "Error: Could not open data file for table '" << tableName << "'." << std::endl;
//             return results;
//         }

//         std::string line;
//         int rowCount = 0;
//         while (std::getline(dataFile, line)) {
//             rowCount++;
//             std::vector<std::string> rowValues = splitString(line, ',');

//             // 验证行格式
//             if (rowValues.size() != columnsInfo.size()) {
//                 std::cerr << "Warning: Skipping malformed row " << rowCount << " (expected "
//                           << columnsInfo.size() << " columns, got " << rowValues.size() << ")" << std::endl;
//                 continue;
//             }

//             // 应用 WHERE 过滤
//             if (whereTree && !evaluateWhereClauseTree(whereTree, rowValues, columnsInfo)) {
//                 continue;
//             }

//             // 生成分组键
//             std::string groupKey;
//             for (const auto& col : groupByColumns) {
//                 int idx = findColumnIndex(columnsInfo, col);
//                 if (idx != -1) {
//                     groupKey += rowValues[idx] + "\t";
//                 }
//             }

//             groupedRows[groupKey].push_back(rowValues);
//         }

//         dataFile.close();

//         // 计算每个分组的聚合结果
//         for (const auto& [groupKey, groupRows] : groupedRows) {
//             std::vector<std::string> resultRow;

//             // 添加分组列的值
//             std::vector<std::string> groupValues = splitString(groupKey, '\t');
//             resultRow.insert(resultRow.end(), groupValues.begin(), groupValues.end());

//             // 计算聚合函数
//             for (const auto& [funcName, param] : parsedAggregateFunctions) {
//                 if (funcName == "COUNT") {
//                     if (param == "*") {
//                         resultRow.push_back(std::to_string(groupRows.size()));
//                     } else {
//                         int colIdx = findColumnIndex(columnsInfo, param);
//                         if (colIdx == -1) {
//                             resultRow.push_back("0");
//                             continue;
//                         }
//                         int count = 0;
//                         for (const auto& row : groupRows) {
//                             if (!row[colIdx].empty()) count++;
//                         }
//                         resultRow.push_back(std::to_string(count));
//                     }
//                 } else if (funcName == "SUM" || funcName == "AVG" || funcName == "MAX" || funcName == "MIN") {
//                     int colIdx = findColumnIndex(columnsInfo, param);
//                     if (colIdx == -1) {
//                         resultRow.push_back("NULL");
//                         continue;
//                     }

//                     std::vector<double> values;
//                     for (const auto& row : groupRows) {
//                         if (!row[colIdx].empty()) {
//                             try {
//                                 values.push_back(std::stod(row[colIdx]));
//                             } catch (...) {
//                                 // 转换失败，忽略该值
//                             }
//                         }
//                     }

//                     if (values.empty()) {
//                         resultRow.push_back(funcName == "AVG" ? "0" : "NULL");
//                         continue;
//                     }

//                     if (funcName == "SUM") {
//                         double sum = 0.0;
//                         for (double val : values) sum += val;
//                         resultRow.push_back(std::to_string(sum));
//                     } else if (funcName == "AVG") {
//                         double sum = 0.0;
//                         for (double val : values) sum += val;
//                         resultRow.push_back(std::to_string(sum / values.size()));
//                     } else if (funcName == "MAX") {
//                         double maxVal = *std::max_element(values.begin(), values.end());
//                         resultRow.push_back(std::to_string(maxVal));
//                     } else if (funcName == "MIN") {
//                         double minVal = *std::min_element(values.begin(), values.end());
//                         resultRow.push_back(std::to_string(minVal));
//                     }
//                 } else {
//                     resultRow.push_back("NULL");
//                 }
//             }

//             results.push_back(resultRow);
//         }
//     } else {
//         // 不带 GROUP BY 的聚合查询
//         std::vector<std::vector<std::string>> filteredRows;

//         // 读取数据文件并应用 WHERE 过滤
//         std::string dataFilePath = "../../res/" + tableName + ".data.txt";
//         std::ifstream dataFile(dataFilePath);
//         if (!dataFile.is_open()) {
//             std::cerr << "Error: Could not open data file for table '" << tableName << "'." << std::endl;
//             return results;
//         }

//         std::string line;
//         int rowCount = 0;
//         while (std::getline(dataFile, line)) {
//             rowCount++;
//             std::vector<std::string> rowValues = splitString(line, ',');

//             // 验证行格式
//             if (rowValues.size() != columnsInfo.size()) {
//                 std::cerr << "Warning: Skipping malformed row " << rowCount << " (expected "
//                           << columnsInfo.size() << " columns, got " << rowValues.size() << ")" << std::endl;
//                 continue;
//             }

//             // 应用 WHERE 过滤
//             if (whereTree && !evaluateWhereClauseTree(whereTree, rowValues, columnsInfo)) {
//                 continue;
//             }

//             filteredRows.push_back(rowValues);
//         }

//         dataFile.close();


//         // 计算全局聚合结果
//         std::vector<std::string> resultRow;
//         for (const auto& [funcName, param] : parsedAggregateFunctions) {
//             if (funcName == "COUNT") {
//                 if (param == "*") {
//                     resultRow.push_back(std::to_string(filteredRows.size()));
//                 } else {
//                     int colIdx = findColumnIndex(columnsInfo, param);
//                     if (colIdx == -1) {
//                         resultRow.push_back("0");
//                         continue;
//                     }
//                     int count = 0;
//                     for (const auto& row : filteredRows) {
//                         if (!row[colIdx].empty()) count++;
//                     }
//                     resultRow.push_back(std::to_string(count));
//                 }
//             } else if (funcName == "SUM" || funcName == "AVG" || funcName == "MAX" || funcName == "MIN") {
//                 int colIdx = findColumnIndex(columnsInfo, param);
//                 if (colIdx == -1) {
//                     resultRow.push_back("NULL");
//                     continue;
//                 }

//                 std::vector<double> values;
//                 for (const auto& row : filteredRows) {
//                     if (!row[colIdx].empty()) {
//                         try {
//                             values.push_back(std::stod(row[colIdx]));
//                         } catch (...) {
//                             // 转换失败，忽略该值
//                         }
//                     }
//                 }

//                 if (values.empty()) {
//                     resultRow.push_back(funcName == "AVG" ? "0" : "NULL");
//                     continue;
//                 }

//                 if (funcName == "SUM") {
//                     double sum = 0.0;
//                     for (double val : values) sum += val;
//                     resultRow.push_back(std::to_string(sum));
//                 } else if (funcName == "AVG") {
//                     double sum = 0.0;
//                     for (double val : values) sum += val;
//                     resultRow.push_back(std::to_string(sum / values.size()));
//                 } else if (funcName == "MAX") {
//                     double maxVal = *std::max_element(values.begin(), values.end());
//                     resultRow.push_back(std::to_string(maxVal));
//                 } else if (funcName == "MIN") {
//                     double minVal = *std::min_element(values.begin(), values.end());
//                     resultRow.push_back(std::to_string(minVal));
//                 }
//             } else {
//                 resultRow.push_back("NULL");
//             }
//         }

//         if (!resultRow.empty()) {
//             results.push_back(resultRow);
//         }
//     }

//     std::cout << "Successfully selected " << results.size() << " rows from table '" << tableName << "'." << std::endl;


//     // 记录查询操作的日志
//     Logger logger("../../res/system_logs.txt");
//     logger.log(Session::getCurrentUserId(), "SELECT", "DATA", "Selected data from " + tableName + " in " + dbName); // 记录日志
//     return results;
// }





std::vector<std::vector<std::string>> datamanager::selecData(
    const std::string& dbName,
    const std::string& tableName,
    const std::vector<std::string>& columnsToSelect,
    const std::shared_ptr<Node>& whereTree,
    const std::vector<std::string>& aggregateFunctions,
    const std::vector<std::string>& groupByColumns

    ) {
    std::vector<std::vector<std::string>> results;

    // 检查输入参数
    std::cout << "[DEBUG] Executor - Input parameters:" << std::endl;
    std::cout << "[DEBUG]   dbName: " << dbName << std::endl;
    std::cout << "[DEBUG]   tableName: " << tableName << std::endl;
    std::cout << "[DEBUG]   columnsToSelect: ";
    for (const auto& col : columnsToSelect) std::cout << col << " ";
    std::cout << std::endl;
    std::cout << "[DEBUG]   aggregateFunctions: ";
    for (const auto& func : aggregateFunctions) std::cout << func << " ";
    std::cout << std::endl;
    std::cout << "[DEBUG]   groupByColumns: ";
    for (const auto& col : groupByColumns) std::cout << col << " ";
    std::cout << std::endl;

    // 1. 验证表是否存在
    tableManage::TableInfo tableInfo = tableMgr.getTableInfo(dbName, tableName);
    if (tableInfo.table_name.empty()) {
        std::cerr << "Error selecting data: Table '" << tableName << "' does not exist in database '" << dbName << "'." << std::endl;
        return results;
    }

    // 2. 获取表的列信息
    std::vector<fieldManage::FieldInfo> columnsInfo = fieldMgr.getFieldsInfo(dbName, tableName);
    if (columnsInfo.empty()) {
        std::cerr << "Error selecting data: Could not retrieve column information for table '" << tableName << "'." << std::endl;
        return results;
    }

    // 3. 确定要选择的列以及它们在数据行中的索引

    std::vector<int> selectColumnIndices;
    bool hasSelectAll = false; // [NEW] 标记是否为 SELECT *

    if (columnsToSelect.empty()) {
        // 未指定列时默认选择所有列
        for (size_t i = 0; i < columnsInfo.size(); ++i) {
            selectColumnIndices.push_back(static_cast<int>(i));
        }
    } else {
        for (const auto& colName : columnsToSelect) {
            if (colName == "*") { // [NEW] 处理 SELECT *
                hasSelectAll = true;
                selectColumnIndices.clear(); // 清空现有列，重新添加所有列索引
                for (size_t i = 0; i < columnsInfo.size(); ++i) {
                    selectColumnIndices.push_back(static_cast<int>(i));
                }
                break; // 遇到 * 时忽略后续列（符合 SQL 语法）
            } else {
                // 检查列是否存在
                int index = findColumnIndex(columnsInfo, colName);
                if (index == -1) {
                    std::cerr << "Error selecting data: Unknown column '" << colName << "' specified in SELECT list for table '" << tableName << "'." << std::endl;
                    return results;
                }
                selectColumnIndices.push_back(index);
            }
        }
    }

    // 解析聚合函数
    std::vector<std::pair<std::string, std::string>> parsedAggregateFunctions;
    for (const auto& func : aggregateFunctions) {
        size_t openPos = func.find('(');
        size_t closePos = func.find(')');
        if (openPos != std::string::npos && closePos != std::string::npos && openPos < closePos) {
            std::string funcName = utils::toUpper(utils::strip(func.substr(0, openPos)));
            std::string param = utils::strip(func.substr(openPos + 1, closePos - openPos - 1));
            parsedAggregateFunctions.emplace_back(funcName, param);
        }
    }

    // 无聚合函数的简单查询
    if (aggregateFunctions.empty()) {
        std::string dataFilePath = "../../res/" + tableName + ".data.txt";
        std::ifstream dataFile(dataFilePath);
        if (!dataFile.is_open()) {
            // 表为空时返回空结果
            return results;
        }

        // 直接跳过第一行（表头）
        std::string headerLine;
        std::getline(dataFile, headerLine);


        std::string line;
        int rowCount = 0;
        while (std::getline(dataFile, line)) {
            rowCount++;
            std::vector<std::string> rowValues = splitString(line, ',');

            // 验证行格式
            if (rowValues.size() != columnsInfo.size()) {
                std::cerr << "Warning: Skipping malformed row " << rowCount << " (expected "
                          << columnsInfo.size() << " columns, got " << rowValues.size() << ")" << std::endl;
                continue;
            }

            // 应用 WHERE 过滤
            if (whereTree && !evaluateWhereClauseTree(whereTree, rowValues, columnsInfo)) {
                continue;
            }

            // 收集结果行
            std::vector<std::string> selectedRow;
            for (int idx : selectColumnIndices) {
                selectedRow.push_back(rowValues[idx]);
            }
            results.push_back(selectedRow);
        }

        dataFile.close();
        return results;
    }

    // 处理聚合查询
    if (!groupByColumns.empty()) {
        // 带 GROUP BY 的聚合查询
        std::unordered_map<std::string, std::vector<std::vector<std::string>>> groupedRows;
        std::string dataFilePath = "../../res/" + tableName + ".data.txt";
        std::ifstream dataFile(dataFilePath);
        if (!dataFile.is_open()) {
            std::cerr << "Error: Could not open data file for table '" << tableName << "'." << std::endl;
            return results;
        }

        // 直接跳过第一行（表头）
        std::string headerLine;
        std::getline(dataFile, headerLine);


        std::string line;
        int rowCount = 0;
        while (std::getline(dataFile, line)) {
            rowCount++;
            std::vector<std::string> rowValues = splitString(line, ',');

            // 验证行格式
            if (rowValues.size() != columnsInfo.size()) {
                std::cerr << "Warning: Skipping malformed row " << rowCount << " (expected "
                          << columnsInfo.size() << " columns, got " << rowValues.size() << ")" << std::endl;
                continue;
            }

            // 应用 WHERE 过滤
            if (whereTree && !evaluateWhereClauseTree(whereTree, rowValues, columnsInfo)) {
                continue;
            }

            // 生成分组键
            std::string groupKey;
            for (const auto& col : groupByColumns) {
                int idx = findColumnIndex(columnsInfo, col);
                if (idx != -1) {
                    groupKey += rowValues[idx] + "\t";
                }
            }

            groupedRows[groupKey].push_back(rowValues);
        }

        dataFile.close();

        // 计算每个分组的聚合结果
        for (const auto& [groupKey, groupRows] : groupedRows) {
            std::vector<std::string> resultRow;

            // 添加分组列的值
            std::vector<std::string> groupValues = splitString(groupKey, '\t');
            resultRow.insert(resultRow.end(), groupValues.begin(), groupValues.end());

            // 计算聚合函数
            for (const auto& [funcName, param] : parsedAggregateFunctions) {
                if (funcName == "COUNT") {
                    if (param == "*") {//count(*)统计行数
                        resultRow.push_back(std::to_string(groupRows.size()));
                    } else {//count(column)统计非空值数量
                        int colIdx = findColumnIndex(columnsInfo, param);
                        if (colIdx == -1) {
                            resultRow.push_back("0");
                            continue;
                        }
                        int count = 0;
                        for (const auto& row : groupRows) {
                            if (!row[colIdx].empty()) count++;
                        }
                        resultRow.push_back(std::to_string(count));
                    }
                } else if (funcName == "SUM" || funcName == "AVG" || funcName == "MAX" || funcName == "MIN") {
                    int colIdx = findColumnIndex(columnsInfo, param);
                    if (colIdx == -1) {
                        resultRow.push_back("NULL");
                        continue;
                    }

                    //转换为数值类型（处理类型转换失败的情况）
                    std::vector<double> values;
                    for (const auto& row : groupRows) {
                        if (!row[colIdx].empty()) {
                            try {
                                values.push_back(std::stod(row[colIdx]));
                            } catch (...) {
                                // 转换失败，忽略该值
                            }
                        }
                    }

                    if (values.empty()) {
                        resultRow.push_back(funcName == "AVG" ? "0" : "NULL");
                        continue;
                    }

                    if (funcName == "SUM") {
                        double sum = 0.0;
                        for (double val : values) sum += val;
                        resultRow.push_back(std::to_string(sum));
                    } else if (funcName == "AVG") {
                        double sum = 0.0;
                        for (double val : values) sum += val;
                        resultRow.push_back(std::to_string(sum / values.size()));
                    } else if (funcName == "MAX") {
                        double maxVal = *std::max_element(values.begin(), values.end());
                        resultRow.push_back(std::to_string(maxVal));
                    } else if (funcName == "MIN") {
                        double minVal = *std::min_element(values.begin(), values.end());
                        resultRow.push_back(std::to_string(minVal));
                    }
                } else {
                    resultRow.push_back("NULL");
                }
            }

            results.push_back(resultRow);
        }
    } else {
        // 不带 GROUP BY 的聚合查询
        std::vector<std::vector<std::string>> filteredRows;
        std::string dataFilePath = "../../res/" + tableName + ".data.txt";
        std::ifstream dataFile(dataFilePath);
        if (!dataFile.is_open()) {
            std::cerr << "Error: Could not open data file for table '" << tableName << "'." << std::endl;
            return results;
        }

        // 直接跳过第一行（表头）
        std::string headerLine;
        std::getline(dataFile, headerLine);

        std::string line;
        int rowCount = 0;
        while (std::getline(dataFile, line)) {
            rowCount++;
            std::vector<std::string> rowValues = splitString(line, ',');

            // 验证行格式
            if (rowValues.size() != columnsInfo.size()) {
                std::cerr << "Warning: Skipping malformed row " << rowCount << " (expected "
                          << columnsInfo.size() << " columns, got " << rowValues.size() << ")" << std::endl;
                continue;
            }

            // 应用 WHERE 过滤
            if (whereTree && !evaluateWhereClauseTree(whereTree, rowValues, columnsInfo)) {
                continue;
            }

            filteredRows.push_back(rowValues);
        }

        dataFile.close();

        // 计算全局聚合结果
        std::vector<std::string> resultRow;
        for (const auto& [funcName, param] : parsedAggregateFunctions) {
            if (funcName == "COUNT") {
                if (param == "*") {
                    resultRow.push_back(std::to_string(filteredRows.size()));
                } else {
                    int colIdx = findColumnIndex(columnsInfo, param);
                    if (colIdx == -1) {
                        resultRow.push_back("0");
                        continue;
                    }
                    int count = 0;
                    for (const auto& row : filteredRows) {
                        if (!row[colIdx].empty()) count++;
                    }
                    resultRow.push_back(std::to_string(count));
                }
            } else if (funcName == "SUM" || funcName == "AVG" || funcName == "MAX" || funcName == "MIN") {
                int colIdx = findColumnIndex(columnsInfo, param);
                if (colIdx == -1) {
                    resultRow.push_back("NULL");
                    continue;
                }

                std::vector<double> values;
                for (const auto& row : filteredRows) {
                    if (!row[colIdx].empty()) {
                        try {
                            values.push_back(std::stod(row[colIdx]));
                        } catch (...) {
                            // 转换失败，忽略该值
                        }
                    }
                }

                if (values.empty()) {
                    resultRow.push_back(funcName == "AVG" ? "0" : "NULL");
                    continue;
                }

                if (funcName == "SUM") {
                    double sum = 0.0;
                    for (double val : values) sum += val;
                    resultRow.push_back(std::to_string(sum));
                } else if (funcName == "AVG") {
                    double sum = 0.0;
                    for (double val : values) sum += val;
                    resultRow.push_back(std::to_string(sum / values.size()));
                } else if (funcName == "MAX") {
                    double maxVal = *std::max_element(values.begin(), values.end());
                    resultRow.push_back(std::to_string(maxVal));
                } else if (funcName == "MIN") {
                    double minVal = *std::min_element(values.begin(), values.end());
                    resultRow.push_back(std::to_string(minVal));
                }
            } else {
                resultRow.push_back("NULL");
            }
        }

        if (!resultRow.empty()) {
            results.push_back(resultRow);
        }
    }



    // 应用 ORDER BY 排序

    std::cout << "Successfully selected " << results.size() << " rows from table '" << tableName << "'." << std::endl;

    // 记录查询操作的日志
    Logger logger("../../res/system_logs.txt");
    logger.log(Session::getCurrentUserId(), "SELECT", "DATA", "Selected data from " + tableName + " in " + dbName);
    return results;
}










// =======
//             selectColumnIndices.push_back(index);
//         }
//     }

//     // 4. WHERE 子句的解析工作已由调用方完成，whereTree 参数直接传入。
//     //    如果 whereTree 是 nullptr，evaluateWhereClauseTree 会处理为不筛选。

//     // 5. 打开数据文件进行读取
//     std::string dataFilePath = "../../res/" + tableName + ".data.txt"; // 构建数据文件路径
//     std::ifstream dataFile(dataFilePath);
//     if (!dataFile.is_open()) {
//         // 如果数据文件不存在，说明表是空的。对于 SELECT 来说这不是错误。
//         // std::cout << "Info: Data file '" << dataFilePath << "' not found. Table is empty." << std::endl; // 可选的信息提示
//         return results; // 返回空结果集
//     }

//     // 6. 逐行读取数据文件，应用 WHERE 子句筛选，并提取所需列
//     std::string line;
//     while (std::getline(dataFile, line)) {
//         if (line.empty()) continue; // 跳过空行

//         // 分割行数据为各个字段值
//         std::vector<std::string> rowValues = splitString(line, ',');

//         // 验证行数据的字段数量是否与表结构一致
//         if (rowValues.size() != columnsInfo.size()) {
//             std::cerr << "Warning: Invalid data row in table '" << tableName
//                       << "': Expected " << columnsInfo.size()
//                       << " fields, but found " << rowValues.size() << " fields. Skipping this row." << std::endl;
//             continue; // 跳过格式错误的行
//         }

//         // 应用 WHERE 子句筛选
//         bool rowMatches = evaluateWhereClauseTree(whereTree, rowValues, columnsInfo);
//         if (!rowMatches) continue; // 如果不满足 WHERE 条件，则跳过该行

//         // 提取所需列的值
//         std::vector<std::string> selectedValues;
//         for (int index : selectColumnIndices) {
//             selectedValues.push_back(rowValues[index]);
//         }

//         // 将筛选后的行添加到结果中
//         results.push_back(selectedValues);
//     }

//     // 7. 关闭文件并返回结果
//     dataFile.close();

//     // 记录查询操作的日志
//     Logger logger("../../res/system_logs.txt");
//     logger.log(Session::getCurrentUserId(), "SELECT", "DATA", "Selected data from " + tableName + " in " + dbName); // 记录日志

//     return results;
// }

// >>>>>>> 9e3794f9e4ed8e4a413a17e5976ebb9162f7bbed
// 执行 UPDATE 操作
bool datamanager::updateData( const std::string& dbName,
                             const std::string& tableName,
                             const std::map<std::string, std::string>& setMap,
                             const std::string& primaryKeyName,
                             const std::string& primaryKeyValue){
    // 1. 获取表结构
    tableManage::TableInfo tableInfo = tableMgr.getTableInfo(dbName, tableName);
    if (tableInfo.table_name.empty()) {
        std::cerr << "Error: Table '" << tableName << "' does not exist." << std::endl;
        return false;
    }

    std::vector<fieldManage::FieldInfo> columnsInfo = fieldMgr.getFieldsInfo(dbName, tableName);
    if (columnsInfo.empty()) {
        std::cerr << "Error: No column info found for table '" << tableName << "'." << std::endl;
        return false;
    }

    // 找主键索引
    int primaryKeyIndex = -1;
    std::map<std::string, int> columnIndexMap;
    for (int i = 0; i < columnsInfo.size(); ++i) {
        columnIndexMap[columnsInfo[i].fieldName] = i;
        if (columnsInfo[i].fieldName == primaryKeyName) {
            primaryKeyIndex = i;
        }
    }

    if (primaryKeyIndex == -1) {
        std::cerr << "Error: Primary key '" << primaryKeyName << "' not found." << std::endl;
        return false;
    }

    std::string dataFilePath = buildFilePath(dbName, tableName);
    std::string tempFilePath = dataFilePath + ".tmp";

    std::ifstream inFile(dataFilePath);
    std::ofstream outFile(tempFilePath);
    if (!inFile.is_open() || !outFile.is_open()) {
        std::cerr << "Error: Failed to open file for update." << std::endl;
        return false;
    }

    std::string line;
    bool updated = false;
    while (std::getline(inFile, line)) {
        std::vector<std::string> rowValues = splitString(line, ',');
        if (rowValues.size() != columnsInfo.size()) {
            outFile << line << "\n"; // 保持原样
            continue;
        }

        if (rowValues[primaryKeyIndex] == primaryKeyValue) {
            for (const auto& [colName, newVal] : setMap) {
                if (columnIndexMap.count(colName)) {
                    rowValues[columnIndexMap[colName]] = newVal;
                }
            }
            updated = true;
        }

        outFile << joinRowValues(rowValues) << "\n";
    }

    inFile.close();
    outFile.close();

    if (!updated) {
        std::cerr << "Warning: No matching row with primary key = " << primaryKeyValue << std::endl;
        std::remove(tempFilePath.c_str());
        return false;
    }

    std::remove(dataFilePath.c_str());
    std::rename(tempFilePath.c_str(), dataFilePath.c_str());

    updateTableLastModifiedDate(dbName, tableName);
    return true;

}




// 实现事务回滚功能
bool datamanager::rollbackTransaction() {
    // 读取 undo 文件
    QFile undoFile("undo.txt");
    if (!undoFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "Failed to open undo file.\n";
        return false;
    }

    QTextStream in(&undoFile);
    QStringList commands;
    while (!in.atEnd()) {
        commands.append(in.readLine());
    }
    undoFile.close();

    // 清空 undo 文件
    if (!undoFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        std::cerr << "Failed to truncate undo file.\n";
        return false;
    }
    undoFile.close();

    // 反转命令顺序
    //commands = commands.reversed();
    // 使用 std::reverse 反转命令顺序
    std::reverse(commands.begin(), commands.end());


    // 执行回滚操作
    for (const QString& command : commands) {
        // 解析命令并执行回滚操作
        std::string cmdStr = command.toStdString();
        if (cmdStr.find("INSERT INTO") == 0) {
            // 处理插入命令的回滚
            // 提取表名和值
            size_t intoPos = cmdStr.find("INTO") + 4;
            size_t valuesPos = cmdStr.find("VALUES");
            std::string tableName = cmdStr.substr(intoPos, valuesPos - intoPos - 1);
            tableName.erase(0, tableName.find_first_not_of(" \t"));
            tableName.erase(tableName.find_last_not_of(" \t") + 1);

            // 提取主键值（假设第一个字段是主键）
            size_t openBracket = cmdStr.find("(");
            size_t closeBracket = cmdStr.find(")");
            std::string valuesStr = cmdStr.substr(openBracket + 1, closeBracket - openBracket - 1);
            std::vector<std::string> values = splitString(valuesStr, ',');
            if (!values.empty()) {
                std::string primaryKeyValue = values[0];
                // 移除首尾的引号
                if (!primaryKeyValue.empty() && primaryKeyValue.front() == '\'') {
                    primaryKeyValue = primaryKeyValue.substr(1);
                }
                if (!primaryKeyValue.empty() && primaryKeyValue.back() == '\'') {
                    primaryKeyValue.pop_back();
                }
                deleteData("", tableName, primaryKeyValue);
            }
        } else if (cmdStr.find("UPDATE") == 0) {
            // 处理更新命令的回滚
            // 这里简化处理，实际应用中需要记录更新前的值
            std::cerr << "Rollback for UPDATE commands is not fully implemented.\n";
        } else if (cmdStr.find("DELETE") == 0) {
            // 处理删除命令的回滚
            // 这里简化处理，实际应用中需要记录删除前的值
            std::cerr << "Rollback for DELETE commands is not fully implemented.\n";
        }
    }

    return true;
}

// 提交事务（清空 undo 文件）
bool datamanager::commitTransaction() {
    QFile undoFile("undo.txt");
    if (!undoFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        std::cerr << "Failed to truncate undo file.\n";
        return false;
    }
    undoFile.close();
    return true;
}

// 获取当前日期时间，格式为 YYYYMMDD_HHMMSS
std::string datamanager::getCurrentDateTime() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::tm tm = *std::localtime(&in_time_t);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &tm);

    return std::string(buffer);
}


// 实现备份功能
bool datamanager::backupDatabase(const std::string& dbName) {
    // 备份数据库中的所有表
    std::vector<tableManage::TableInfo> tables = tableMgr.getTablesInDatabase(dbName); // 获取表列表
    if (tables.empty()) {
        std::cerr << "No tables to backup in database " << dbName << std::endl;
        return false;
    }

    // 创建备份目录（如果不存在）
    std::string backupDir = "../../res/backup/" + dbName + "_" + getCurrentDateTime();
    if (!std::filesystem::exists(backupDir)) {
        if (!std::filesystem::create_directories(backupDir)) {
            std::cerr << "Failed to create backup directory: " << backupDir << std::endl;
            return false;
        }
    }

    // 备份每个表的数据文件和表结构文件
    for (const auto& table : tables) {
        std::string tableName = table.table_name;

        // 备份数据文件
        std::string dataFilePath = buildFilePath(dbName, tableName);
        std::string backupDataFilePath = backupDir + "/" + tableName + ".data.txt";

        if (std::filesystem::exists(dataFilePath)) {
            try {
                std::filesystem::copy_file(dataFilePath, backupDataFilePath, std::filesystem::copy_options::overwrite_existing);
            } catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "Failed to backup data file " << dataFilePath << ": " << e.what() << std::endl;
                return false;
            }
        } else {
            std::cout << "Data file " << dataFilePath << " does not exist, skipping." << std::endl;
        }

        // 备份表结构文件
        std::string tableDescFile = "../../res/" + dbName + ".tb.txt";
        std::string backupTableDescFile = backupDir + "/" + dbName + ".tb.txt";

        if (std::filesystem::exists(tableDescFile)) {
            try {
                std::filesystem::copy_file(tableDescFile, backupTableDescFile, std::filesystem::copy_options::overwrite_existing);
            } catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "Failed to backup table description file " << tableDescFile << ": " << e.what() << std::endl;
                return false;
            }
        } else {
            std::cout << "Table description file " << tableDescFile << " does not exist, skipping." << std::endl;
        }
    }

    std::cout << "Database " << dbName << " backed up successfully to " << backupDir << std::endl;
    return true;
}


// 实现恢复功能
bool datamanager::restoreDatabase(const std::string& dbName, const std::string& backupPath) {
    // 检查备份路径是否存在
    if (!std::filesystem::exists(backupPath)) {
        std::cerr << "Backup path does not exist: " << backupPath << std::endl;
        return false;
    }

    // 获取备份中的所有表文件
    std::vector<std::string> tableFiles;
    for (const auto& entry : std::filesystem::directory_iterator(backupPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".data.txt") {
            tableFiles.push_back(entry.path().filename().string());
        }
    }

    if (tableFiles.empty()) {
        std::cerr << "No table data files found in backup: " << backupPath << std::endl;
        return false;
    }

    // 恢复每个表的数据文件
    for (const auto& tableFile : tableFiles) {
        std::string tableName = tableFile.substr(0, tableFile.find(".data.txt"));

        // 构建源文件路径和目标文件路径
        std::string sourceFilePath = backupPath + "/" + tableFile;
        std::string targetFilePath = buildFilePath(dbName, tableName);

        // 检查目标表是否存在
        tableManage::TableInfo tableInfo = tableMgr.getTableInfo(dbName, tableName);
        if (tableInfo.table_name.empty()) {
            std::cerr << "Table " << tableName << " does not exist in database " << dbName << std::endl;
            continue; // 跳过不存在的表
        }

        // 恢复数据文件
        try {
            std::filesystem::copy_file(sourceFilePath, targetFilePath, std::filesystem::copy_options::overwrite_existing);
            std::cout << "Restored table " << tableName << " from " << sourceFilePath << std::endl;
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Failed to restore table " << tableName << ": " << e.what() << std::endl;
            return false;
        }
    }

    // 恢复表结构文件
    std::string sourceTableDescFile = backupPath + "/" + dbName + ".tb.txt";
    std::string targetTableDescFile = "../../res/" + dbName + ".tb.txt";

    if (std::filesystem::exists(sourceTableDescFile)) {
        try {
            std::filesystem::copy_file(sourceTableDescFile, targetTableDescFile, std::filesystem::copy_options::overwrite_existing);
            std::cout << "Restored table description file from " << sourceTableDescFile << std::endl;
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Failed to restore table description file: " << e.what() << std::endl;
            return false;
        }
    } else {
        std::cout << "Table description file not found in backup, skipping." << std::endl;
    }

    std::cout << "Database " << dbName << " restored successfully from " << backupPath << std::endl;
    return true;
}

// 析构函数
datamanager::~datamanager() {
    // 确保事务已提交
    commitTransaction();
}
