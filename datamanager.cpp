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
#include <cctype>
#include "lexer.h"
namespace fs=std::filesystem;

datamanager::datamanager(dbManager* dbMgr) : dbMgr(dbMgr), affair("undo.txt",nullptr) {
    // 构造函数初始化 affair
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
bool datamanager::convertValueForComparison(
    const std::string& valueStr,         // 要转换的字符串值
    const std::string& columnType,       // 列的数据类型
    int& intVal, double& doubleVal, bool& boolVal, std::string& stringVal // 输出参数，存储转换结果
    ) {
    std::string typeUpper = columnType;
    std::transform(typeUpper.begin(), typeUpper.end(), typeUpper.begin(), ::toupper); // 转换为大写以便比较类型

    // 处理从解析器中来的字符串字面值，如果带有单引号则去除
    // 注意：这个处理逻辑依赖于您的 Lexer 如何处理字符串字面值。
    // 如果 Lexer 在解析时已经去除了单引号，这里就不需要了。
    std::string cleanedValueStr = valueStr;
    if (cleanedValueStr.size() >= 2 && cleanedValueStr.front() == '\'' && cleanedValueStr.back() == '\'') {
        cleanedValueStr = cleanedValueStr.substr(1, cleanedValueStr.size() - 2);
    }

    if (typeUpper == "INT") {
        return tryStringtoInt(cleanedValueStr, intVal); // 尝试转换为 INT
    } else if (typeUpper == "DOUBLE" || typeUpper == "FLOAT") {
        return tryStringtoDouble(cleanedValueStr, doubleVal); // 尝试转换为 DOUBLE/FLOAT
    } else if (typeUpper == "BOOL" || typeUpper == "BOOLEAN") {
        return tryStringtoBool(cleanedValueStr, boolVal); // 尝试转换为 BOOL
    } else if (typeUpper == "VARCHAR" || typeUpper == "TEXT") {
        stringVal = cleanedValueStr; // 对于字符串类型，直接存储处理过的字符串
        return true; // 字符串本身转换为字符串总是成功的
    }
    // 如果有其他数据类型（如 DATE, TIME 等），在这里添加转换逻辑

    // 如果遇到未知类型，返回 false 表示转换失败
    return false;
}

// --- 求值 WHERE 子句的 AST 函数 ---
// 递归函数，用于判断一行数据是否符合 WHERE 子句的条件树
bool datamanager::evaluateWhereClauseTree(
    const std::shared_ptr<Node>& whereTree,                // WHERE 子句的 AST 根节点 (可能为 nullptr)
    const std::vector<std::string>& rowValues,           // 当前行的数据值 (字符串向量)
    const std::vector<fieldManage::FieldInfo>& columnsInfo // 表的所有列信息
    ) {
    if (!whereTree) {
        return true; // 如果 AST 为空 (例如 WHERE 子句为空 或 解析失败), 则认为该行符合条件 (即不筛选)
    }

    // 直接在解引用后的Node对象上使用std::holds_alternative
    if (std::holds_alternative<Condition>(*whereTree)) {
        // 直接在解引用后的Node对象上使用std::get
        const auto& cond = std::get<Condition>(*whereTree);

        // 查找条件中列的索引
        int colIndex = findColumnIndex(columnsInfo, cond.column);
        if (colIndex == -1) {
            std::cerr << "Error evaluating WHERE tree: Unknown column '" << cond.column << "'" << std::endl;
            // 对未知列的条件，认为该行不符合条件
            return false;
        }

        // 检查列索引是否超出当前行的范围 (用于防御性编程，理论上不应该发生，因为行字段数应与列信息匹配)
        if (colIndex >= rowValues.size()) {
            std::cerr << "Error evaluating WHERE tree: Column index " << colIndex << " out of bounds for row with " << rowValues.size() << " values." << std::endl;
            return false;
        }

        // 获取列的数据类型以及当前行该列的值和条件中的值
        const std::string& columnType = columnsInfo[colIndex].fieldType;
        const std::string& rowValueStr = rowValues[colIndex];
        const std::string& clauseValueStr = cond.value; // 从 AST 节点获取条件值 (字符串)

        // 将行值和条件值根据列类型进行转换，以便进行类型安全的比较
        int rowInt, clauseInt;
        double rowDouble, clauseDouble;
        bool rowBool, clauseBool;
        std::string rowString, clauseString; // 用于 VARCHAR/TEXT

        // 尝试转换当前行的值
        bool rowConversionOk = convertValueForComparison(rowValueStr, columnType, rowInt, rowDouble, rowBool, rowString);

        // 尝试转换条件中的值
        bool clauseConversionOk = convertValueForComparison(clauseValueStr, columnType, clauseInt, clauseDouble, clauseBool, clauseString); // 注意: 传递 clauseValueStr

        // 如果任何一个值根据列类型转换失败，则认为条件不满足
        if (!rowConversionOk || !clauseConversionOk) {
            // 如果非空值转换失败，输出警告。空字符串尝试转换为非字符串类型会失败，这通常是正确的行为。
            if (!rowValueStr.empty() || !clauseValueStr.empty()){
                std::cerr << "Warning: Type conversion failed during WHERE tree evaluation for column '" << cond.column
                          << "' (type: " << columnType << ") comparing row value '" << rowValueStr
                          << "' with clause value '" << clauseValueStr << "'. Condition is false." << std::endl;
            }
            return false; // 条件不满足，因为值类型不匹配或转换失败
        }

        // 执行基于类型和运算符的比较
        std::string opUpper = cond.op;
        std::transform(opUpper.begin(), opUpper.end(), opUpper.begin(), ::toupper); // 运算符转换为大写

        std::string typeUpper = columnType; // 获取大写后的类型字符串
        std::transform(typeUpper.begin(), typeUpper.end(), typeUpper.begin(), ::toupper);

        if (typeUpper == "INT") {
            if (opUpper == "=" || opUpper == "==") return rowInt == clauseInt;
            if (opUpper == "!=") return rowInt != clauseInt;
            if (opUpper == ">") return rowInt > clauseInt;
            if (opUpper == "<") return rowInt < clauseInt;
            if (opUpper == ">=") return rowInt >= clauseInt;
            if (opUpper == "<=") return rowInt <= clauseInt;
        } else if (typeUpper == "DOUBLE" || typeUpper == "FLOAT") { // 使用 typeUpper
            // 浮点数比较可能需要考虑精度，这里使用精确比较
            if (opUpper == "=" || opUpper == "==") return rowDouble == clauseDouble;
            if (opUpper == "!=") return rowDouble != clauseDouble;
            if (opUpper == ">") return rowDouble > clauseDouble;
            if (opUpper == "<") return rowDouble < clauseDouble;
            if (opUpper == ">=") return rowDouble >= clauseDouble;
            if (opUpper == "<=") return rowDouble <= clauseDouble;
        } else if (typeUpper == "BOOL" || typeUpper == "BOOLEAN") { // 使用 typeUpper
            if (opUpper == "=" || opUpper == "==") return rowBool == clauseBool;
            if (opUpper == "!=") return rowBool != clauseBool;
            // 布尔类型通常不支持 >,<,>=,<= 这样的比较
            // std::cerr << "Warning: Comparison operator '" << cond.op << "' used with BOOL column '" << cond.column << "'. Only = and != are supported. Condition is false." << std::endl; // 可选警告
            return false; // 布尔类型使用了不支持的运算符
        } else if (typeUpper == "VARCHAR" || typeUpper == "TEXT") { // 使用 typeUpper
            if (opUpper == "=" || opUpper == "==") return rowString == clauseString;
            if (opUpper == "!=") return rowString != clauseString;
            // 字符串也支持字典序比较 >,<,>=,<=
            if (opUpper == ">") return rowString > clauseString;
            if (opUpper == "<") return rowString < clauseString;
            if (opUpper == ">=") return rowString >= clauseString;
            if (opUpper == "<=") return rowString <= clauseString;
        }
        // 如果有其他数据类型，在这里添加相应的比较逻辑

        std::cerr << "Error evaluating WHERE tree: Unsupported operator '" << cond.op << "' for column type '" << columnType << "'. Condition is false." << std::endl;
        return false; // 未知的运算符或类型组合
    } else if (std::holds_alternative<LogicalOp>(*whereTree)) {
        // 当前节点是一个逻辑运算符 (LogicalOp)
        const auto& logOp = std::get<LogicalOp>(*whereTree);

        // 递归求值左子树
        bool leftResult = evaluateWhereClauseTree(logOp.left, rowValues, columnsInfo);

        // 根据逻辑运算符执行短路求值 (Short-circuit evaluation)
        std::string opUpper = logOp.op;
        std::transform(opUpper.begin(), opUpper.end(), opUpper.begin(), ::toupper);

        if (opUpper == "AND") {
            if (!leftResult) return false; // 如果左边为 false，AND 结果一定是 false，无需计算右边
        } else if (opUpper == "OR") {
            if (leftResult) return true; // 如果左边为 true，OR 结果一定是 true，无需计算右边
        } else {
            // 如果解析器正确，不应该出现未知的逻辑运算符
            std::cerr << "Internal Error: Unknown logical operator '" << logOp.op << "' in WHERE tree." << std::endl;
            return false; // 未知的逻辑运算符
        }

        // 如果没有短路，则递归求值右子树并合并结果
        bool rightResult = evaluateWhereClauseTree(logOp.right, rowValues, columnsInfo);

        if (opUpper == "AND") {
            return leftResult && rightResult; // 实际执行 AND (如果没短路，leftResult 必为 true)
        } else if (opUpper == "OR") {
            return leftResult || rightResult; // 实际执行 OR (如果没短路，leftResult 必为 false)
        }
        // 已经处理了所有情况，这里的 return 不会执行到
        return false;
    }

    // 如果节点既不是 Condition 也不是 LogicalOp，说明 AST 结构有问题 (不应该发生)
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
    std::string dataFilePath = buildFilePath(dbName, tableName);

    // 如果数据文件不存在，先创建一个空文件
    std::ifstream checkFile(dataFilePath);
    if (!checkFile.good()) {
        std::ofstream createFile(dataFilePath); // 创建空文件
        if (!createFile.is_open()) {
            std::cerr << "Failed to create data file: " << dataFilePath << std::endl;
            return false;
        }
        createFile.close();
        std::cout << "[DEBUG] Data file created: " << dataFilePath << std::endl;
    }

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

// 执行 SELECT 查询
std::vector<std::vector<std::string>> datamanager::selectData(
    const std::string& dbName,
    const std::string& tableName,
    const std::vector<std::string>& columnsToSelect, // 要选择的列名列表 (空表示选择所有列)
    const std::shared_ptr<Node>& whereTree // WHERE 子句的 AST 根节点 (由调用方解析并传入，可能为 nullptr)
    ) {
    std::vector<std::vector<std::string>> results; // 存储查询结果

    // 1. 验证表是否存在
    tableManage::TableInfo tableInfo = tableMgr.getTableInfo(dbName, tableName);
    if (tableInfo.table_name.empty()) {
        std::cerr << "Error selecting data: Table '" << tableName << "' does not exist in database '" << dbName << "'." << std::endl;
        return results; // 表不存在，返回空结果
    }

    // 2. 获取表的列信息
    std::vector<fieldManage::FieldInfo> columnsInfo = fieldMgr.getFieldsInfo(dbName, tableName);
    if (columnsInfo.empty()) {
        std::cerr << "Error selecting data: Could not retrieve column information for table '" << tableName << "'." << std::endl;
        // 即使是空表也应该有列信息，这是一个错误
        return results; // 获取列信息失败，返回空结果
    }

    // 3. 确定要选择的列以及它们在数据行中的索引
    std::vector<int> selectColumnIndices; // 存储要选择的列的索引
    if (columnsToSelect.empty()) {
        // 如果未指定列，则选择所有列
        for (size_t i = 0; i < columnsInfo.size(); ++i) {
            selectColumnIndices.push_back(static_cast<int>(i));
        }
    } else {
        // 如果指定了列，则查找这些列的索引
        for (const auto& colName : columnsToSelect) {
            int index = findColumnIndex(columnsInfo, colName);
            if (index == -1) {
                std::cerr << "Error selecting data: Unknown column '" << colName << "' specified in SELECT list for table '" << tableName << "'." << std::endl;
                return results; // SELECT 列表中包含未知列，返回空结果并报错
            }
            selectColumnIndices.push_back(index);
        }
    }

    // 4. WHERE 子句的解析工作已由调用方完成，whereTree 参数直接传入。
    //    如果 whereTree 是 nullptr，evaluateWhereClauseTree 会处理为不筛选。

    // 5. 打开数据文件进行读取
    std::string dataFilePath = "../../res/" + tableName + ".data.txt"; // 构建数据文件路径
    std::ifstream dataFile(dataFilePath);
    if (!dataFile.is_open()) {
        // 如果数据文件不存在，说明表是空的。对于 SELECT 来说这不是错误。
        // std::cout << "Info: Data file '" << dataFilePath << "' not found. Table is empty." << std::endl; // 可选的信息提示
        return results; // 返回空结果集
    }

    // 6. 逐行读取数据文件，应用 WHERE 子句筛选，并提取所需列
    std::string line;
    while (std::getline(dataFile, line)) {
        if (line.empty()) continue; // 跳过空行

        // 分割行数据为各个字段值
        std::vector<std::string> rowValues = splitString(line, ',');

        // 验证行数据的字段数量是否与表结构一致
        if (rowValues.size() != columnsInfo.size()) {
            std::cerr << "Warning: Invalid data row in table '" << tableName
                      << "': Expected " << columnsInfo.size()
                      << " fields, but found " << rowValues.size() << " fields. Skipping this row." << std::endl;
            continue; // 跳过格式错误的行
        }

        // 应用 WHERE 子句筛选
        bool rowMatches = evaluateWhereClauseTree(whereTree, rowValues, columnsInfo);
        if (!rowMatches) continue; // 如果不满足 WHERE 条件，则跳过该行

        // 提取所需列的值
        std::vector<std::string> selectedValues;
        for (int index : selectColumnIndices) {
            selectedValues.push_back(rowValues[index]);
        }

        // 将筛选后的行添加到结果中
        results.push_back(selectedValues);
    }

    // 7. 关闭文件并返回结果
    dataFile.close();

    // 记录查询操作的日志
    Logger logger("../../res/system_logs.txt");
    logger.log(Session::getCurrentUserId(), "SELECT", "DATA", "Selected data from " + tableName + " in " + dbName); // 记录日志

    return results;
}

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
