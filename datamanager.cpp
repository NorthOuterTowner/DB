#include "datamanager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <QFile>
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
datamanager::datamanager() {}

// 示例函数，用于构建文件路径
std::string buildFilePath(const std::string& dbName, const std::string& tableName) {
    fs::path basePath = fs::current_path() / "res";
    fs::path filePath = basePath / (tableName + ".data.txt");
    std::cout << "Trying to open file: " << filePath.string() << std::endl;
    return filePath.string();
}

bool datamanager::tryStringtoInt(const std::string& s,int& out){
    if(s.empty())return false;
    try{
        size_t pos=0;
        out=std::stoi(s,&pos);
        //检查是否整个字符串都被转换了 并且没有剩余非数字字符
        return pos==s.size();
    }catch (const std::invalid_argument& ia){
        //无效参数 不是一个有效的数字格式
        return false;
    }catch(const std::out_of_range& oor){
        //超出int范围
        return false;
    }
}

bool datamanager::tryStringtoDouble(const std::string& s,double& out){
    if(s.empty())return false;
    try{
        size_t pos=0;
        out = std::stod(s,&pos);
        //检查是否整个字符串都被替换了
        return pos==s.size();
    }catch(const std::invalid_argument& ia){
        return false;
    }catch (const std::out_of_range& oor){
        return false;
    }
}

bool datamanager::tryStringtoBool(const std::string& s,bool& out){
    //简化处理：接收 true、false、1、0
    //需要一个非const的副本进行转换
    std::string upper_s=s;//创建一个非const的副本
    //将副本转换为大写
    std::transform(upper_s.begin(),upper_s.end(),upper_s.begin(),[](unsigned char c){
        return std::toupper(c);
    });

    //进行大写比较
    if(upper_s=="TRUE"||upper_s =="1"){
        out=true;
        return true;
    }else if(upper_s=="FALSE"||upper_s=="0"){
        out=false;
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

//实现 updateTableRecordCount方法
void datamanager::updateTableRecordCount(const std::string& dbName,const std::string& tableName,int count){
    tableMgr.updateTableRecordCount(dbName,tableName,count);
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

bool datamanager::insertData(const std::string& dbName,const std::string& tableName,const std::vector<std::string>& values){
    // 获取表信息
    tableManage::TableInfo tableInfo = tableMgr.getTableInfo(dbName, tableName);
    if (tableInfo.table_name.empty()) {
        std::cerr << "Table " << tableName << " does not exist in database " << dbName << std::endl;
        return false;
    }

    //获取表的详细列信息
    std::vector<fieldManage::FieldInfo>columnsInfo=fieldMgr.getFieldsInfo(tableName);

    // 验证插入数据
    if (!validateInsertData(tableInfo,columnsInfo, values)) {
        std::cerr << "Invalid data for table " << tableName << std::endl;
        return false;
    }

    // 构建数据文件路径
   // std::string dataFilePath = "../../res/" + tableName + ".data.txt";

    std::string dataFilePath = buildFilePath(dbName, tableName);


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
    updateTableRecordCount(dbName, tableName, 1);

    // 更新表的最后修改时间
    updateTableLastModifiedDate(dbName, tableName);

    return true;
}


bool datamanager::validateInsertData(const tableManage::TableInfo& tableInfo,const std::vector<fieldManage::FieldInfo>& columnsInfo, const std::vector<std::string>& values){

    //检查值的数量是否与列数一致
    if(values.size()!=columnsInfo.size()){
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
        bool isNotNull = (column.constraints.find("NOT NULL")!=std::string::npos);
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
            //添加更多数据类型的检查，例如 DATE, TIME 等...
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



bool datamanager::deleteData(const std::string& dbName, const std::string& tableName, const std::vector<int>& rowIndicesToDelete) {
    // 获取表信息，验证表是否存在
    tableManage::TableInfo tableInfo = tableMgr.getTableInfo(dbName, tableName);
    if (tableInfo.table_name.empty()) {
        std::cerr << "Error deleting data: Table '" << tableName << "' does not exist in database '" << dbName << "'." << std::endl;
        return false;
    }

    // 如果没有指定要删除的行，直接返回成功
    if (rowIndicesToDelete.empty()) {
        std::cout << "No rows specified for deletion in table '" << tableName << "'." << std::endl;
        return true;
    }

    // 构建数据文件和临时文件路径
    std::string dataFilePath = "../../res/" + tableName + ".data.txt";
    std::string tempFilePath = dataFilePath + ".tmp"; // 临时文件名称

    // 对要删除的行索引进行排序，方便后续比对
    std::vector<int> sortedIndices = rowIndicesToDelete;
    std::sort(sortedIndices.begin(), sortedIndices.end());

    // 检查是否有重复的索引
    auto it_unique = std::unique(sortedIndices.begin(), sortedIndices.end());
    if (it_unique != sortedIndices.end()) {
        std::cerr << "Warning: Duplicate row indices provided for deletion. Processing unique indices only." << std::endl;
        sortedIndices.erase(it_unique, sortedIndices.end());
    }

    // 打开原始数据文件进行读取
    std::ifstream dataFile(dataFilePath);
    if (!dataFile.is_open()) {
        std::cerr << "Error deleting data: Failed to open data file '" << dataFilePath << "' for reading." << std::endl;
        // 如果文件不存在，并且请求删除一些行，这通常被认为是错误。
        return false;
    }

    // 打开临时文件进行写入
    std::ofstream tempFile(tempFilePath);
    if (!tempFile.is_open()) {
        std::cerr << "Error deleting data: Failed to create temporary file '" << tempFilePath << "' for writing." << std::endl;
        dataFile.close(); // 创建临时文件失败，关闭输入文件
        return false;
    }

    std::string line;
    int currentLineIndex = 0; // 当前正在读取的行索引 (从 0 开始)
    size_t deleteIndexPtr = 0; // 指向 sortedIndices 中当前要比对的索引

    // 逐行读取原始文件，并写入临时文件，跳过需要删除的行
    int deleteCount=0;//记录实际删除的行数

    while (std::getline(dataFile, line)) {
        bool shouldDelete = false;

        // 检查 sortedIndices 中当前指向的索引是否等于当前行索引
        // deleteIndexPtr < sortedIndices.size() 避免越界访问
        if (deleteIndexPtr < sortedIndices.size() && sortedIndices[deleteIndexPtr] == currentLineIndex) {
            shouldDelete = true;
            // 如果当前行需要删除，则前进到 sortedIndices 中的下一个索引
            deleteIndexPtr++;
        }


        if (!shouldDelete) {
            // 如果不需要删除，则写入临时文件
            tempFile << line << std::endl;
        } else {
            deleteCount++;//如果当前行需要删除 则前进到sortedIndices中的下一个索引
        }

        currentLineIndex++; // 移动到下一行，增加行索引
    }

    // 关闭文件
    dataFile.close();
    tempFile.close();

    // 检查临时文件是否写入成功
    if (tempFile.fail()) {
        std::cerr << "Error deleting data: Failed writing to temporary file '" << tempFilePath << "'." << std::endl;
        // 清理部分写入的临时文件
        try { fs::remove(tempFilePath); } catch(const fs::filesystem_error & e) {
            std::cerr << "Error cleaning up temporary file: " << e.what() << std::endl;
        }
        return false;
    }


    // 替换原始数据文件
    try {
        // 删除原始文件
        fs::remove(dataFilePath);

        // 将临时文件重命名为原始文件名称
        fs::rename(tempFilePath, dataFilePath);

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error deleting data: Filesystem error during file replacement: " << e.what() << std::endl;
        // 如果重命名失败，尝试清理临时文件
        try { fs::remove(tempFilePath); } catch(const fs::filesystem_error& e2) {
            std::cerr << "Error cleaning up temporary file after rename failure: " << e2.what() << std::endl;
        }
        return false;
    }

    // 更新元数据
    if(deleteCount>0){//实际成功删除的行数
    updateTableRecordCount(dbName, tableName, -deleteCount); // 减少记录数
    updateTableLastModifiedDate(dbName, tableName); // 更新最后修改时间
    }
    std::cout << "Successfully deleted " << deleteCount << " rows from table '" << tableName << "'." << std::endl;
    return true;
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
    std::vector<fieldManage::FieldInfo> columnsInfo = fieldMgr.getFieldsInfo(tableName);
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
        return results; // 返回空结果 (正确处理了空表的情况)
    }

    // 6. 逐行处理数据文件
    std::string line;
    int rowCount = 0; // 用于行号提示错误
    while (std::getline(dataFile, line)) { // 逐行读取
        rowCount++;
        std::vector<std::string> rowValues = splitString(line, ','); // 将行按逗号分割成各个字段的值

        // 在处理前，检查当前行的字段数量是否与表的列数一致，避免处理格式错误的数据行
        if (rowValues.size() != columnsInfo.size()) {
            std::cerr << "Warning: Skipping row " << rowCount << " in '" << tableName << ".data.txt' due to unexpected number of columns ("
                      << rowValues.size() << " instead of " << columnsInfo.size() << "). This row cannot be evaluated." << std::endl;
            continue; // 跳过处理此行
        }


        // 7. 使用传入的 WHERE AST 求值当前行
        if (evaluateWhereClauseTree(whereTree, rowValues, columnsInfo)) {
            // 8. 如果 evaluateWhereClauseTree 返回 true，表示当前行符合 WHERE 条件，则提取指定列
            std::vector<std::string> selectedRow; // 存储当前行中被选中的列的值
            for (int index : selectColumnIndices) { // 遍历要选择的列的索引
                // 由于前面的检查，index < rowValues.size() 在处理格式正确的行时应该总是成立
                if (index < rowValues.size()) { // 防御性检查
                    selectedRow.push_back(rowValues[index]); // 添加指定列的值
                } else {
                    // 内部错误，列索引越界
                    std::cerr << "Internal Error: Column index " << index << " out of bounds during selection for row " << rowCount << "." << std::endl;
                    selectedRow.push_back(""); // 添加一个占位符或根据需求处理错误
                }
            }
            results.push_back(selectedRow); // 将提取出的行添加到结果集中
        }
    }

    // 9. 关闭数据文件
    if (dataFile.is_open()) {
        dataFile.close();
    }

    std::cout << "Successfully selected " << results.size() << " rows from table '" << tableName << "'." << std::endl;
    return results; // 返回查询结果
}


// 执行 UPDATE 操作
bool datamanager::updateData(
    const std::string& dbName,
    const std::string& tableName,
    const std::string& setClause,    // SET 子句字符串 (例如 "column1=value1, column2=value2")
    const std::shared_ptr<Node>& whereTree  // WHERE 子句的 AST 根节点 (由调用方解析并传入，可能为 nullptr)
    ) {
    // 1. 验证表是否存在
    tableManage::TableInfo tableInfo = tableMgr.getTableInfo(dbName, tableName);
    if (tableInfo.table_name.empty()) {
        std::cerr << "Error updating data: Table '" << tableName << "' does not exist in database '" << dbName << "'." << std::endl;
        return false; // 表不存在，更新失败
    }

    // 2. 获取表的列信息
    std::vector<fieldManage::FieldInfo> columnsInfo = fieldMgr.getFieldsInfo(tableName);
    if (columnsInfo.empty()) {
        std::cerr << "Error updating data: Could not retrieve column information for table '" << tableName << "'." << std::endl;
            // 应该有列信息，这是一个错误
        return false;
    }


    // 3. 解析 SET 子句
    std::map<int, std::string> updates = parseSetClause(setClause, columnsInfo); // 调用辅助函数解析 SET 子句
    if (updates.empty() && !setClause.empty()) {
        // 如果 parseSetClause 返回空 map，但 setClause 不为空，说明 SET 子句解析或验证失败
        std::cerr << "Error updating data: Failed to parse or validate SET clause." << std::endl;
        return false; // SET 子句无效，更新失败
    }
    if (updates.empty()) {
        // 如果 SET 子句为空或解析后没有有效的更新对 (例如 SET col = , )
        std::cerr << "Warning: No valid columns specified in SET clause for update. Update operation cancelled." << std::endl;
        // 没有要更新的内容，但语法上是有效的。这里选择返回 false，因为期望的更新操作没有执行。
        return false;
    }


    // 4. WHERE 子句的解析工作已由调用方完成，whereTree 参数直接传入。
    //    如果 whereTree 是 nullptr，evaluateWhereClauseTree 会处理为更新所有行。

    // 5. 构建数据文件和临时文件路径
    std::string dataFilePath = "../../res/" + tableName + ".data.txt"; // 原始数据文件路径
    std::string tempFilePath = dataFilePath + ".tmp"; // 临时文件路径

    // 6. 打开原始数据文件进行读取
    std::ifstream dataFile(dataFilePath);
    if (!dataFile.is_open()) {
        // 如果数据文件不存在，没有行可以更新。这不是错误，只是没有行被更新。
        std::cout << "Info: Data file '" << dataFilePath << "' not found. No rows to update." << std::endl; // 信息提示
        return true; // 成功完成 (0 行被更新)
    }

    // 7. 创建并打开临时文件进行写入
    std::ofstream tempFile(tempFilePath);
    if (!tempFile.is_open()) {
        std::cerr << "Error updating data: Failed to create temporary file '" << tempFilePath << "' for writing." << std::endl;
        dataFile.close(); // 创建临时文件失败，关闭输入文件
        return false; // 创建文件失败，更新失败
    }

    // 8. 逐行处理原始文件，写入临时文件
    std::string line;
    int updatedRowCount = 0; // 记录实际更新的行数
    int totalRowCount = 0;   // 记录总行数 (用于错误提示)

    while (std::getline(dataFile, line)) { // 逐行读取原始文件
        totalRowCount++;
        std::vector<std::string> rowValues = splitString(line, ','); // 分割当前行的字段值

        // 在处理前，检查当前行的字段数量是否与表的列数一致，避免处理格式错误的数据行
        if (rowValues.size() != columnsInfo.size()) {
            std::cerr << "Warning: Skipping processing for row " << totalRowCount << " in '" << tableName << ".data.txt' due to unexpected number of columns ("
                      << rowValues.size() << " instead of " << columnsInfo.size() << "). This row will be copied as is and cannot be updated." << std::endl;
            tempFile << line << std::endl; // 将格式错误的原样复制到临时文件
            continue; // 跳过此行的更新逻辑
        }

        // 9. 使用传入的 WHERE AST 判断当前行是否需要更新
        if (evaluateWhereClauseTree(whereTree, rowValues, columnsInfo)) {
            // 10. 如果 evaluateWhereClauseTree 返回 true，表示当前行符合 WHERE 条件，需要更新
            std::vector<std::string> updatedRowValues = rowValues; // 复制原始行值，在此基础上修改

            for (const auto& pair : updates) { // 遍历 SET 子句解析出的更新对
                int colIndex = pair.first;     // 要更新的列的索引
                const std::string& newValue = pair.second; // 新值 (字符串形式)

                // colIndex 在 parseSetClause 中已经验证过是否小于 columnsInfo.size()
                // rowValues.size() 在上面已经验证过是否等于 columnsInfo.size()
                // 所以理论上 colIndex 应该小于 updatedRowValues.size()
                if (colIndex < updatedRowValues.size()) { // 防御性检查
                    updatedRowValues[colIndex] = newValue; // 应用更新：将新值赋给对应列
                } else {
                    // 内部错误，更新索引越界
                    std::cerr << "Internal Error: Update column index " << colIndex << " out of bounds when applying SET clause for row " << totalRowCount << "." << std::endl;
                    // 记录错误，但继续处理当前行和后续行
                }
            }

            // 11. 将更新后的行写入临时文件
            tempFile << joinRowValues(updatedRowValues) << std::endl; // 将修改后的行连接成字符串并写入文件
            updatedRowCount++; // 更新计数器

        } else {
            // 12. 如果 evaluateWhereClauseTree 返回 false，表示当前行不符合 WHERE 条件，不需要更新
            tempFile << line << std::endl; // 将原始行原样复制到临时文件
        }
    }

    // 13. 关闭文件
    dataFile.close(); // 关闭原始文件
    tempFile.close(); // 关闭临时文件

    // 检查临时文件是否成功写入
    if (tempFile.fail()) {
        std::cerr << "Error updating data: Failed writing to temporary file '" << tempFilePath << "'." << std::endl;
        // 尝试清理部分写入的临时文件
        try { fs::remove(tempFilePath); } catch(const fs::filesystem_error& e) {
            std::cerr << "Error cleaning up temporary file: " << e.what() << std::endl;
        }
        return false; // 写入失败，更新失败
    }


    // 14. 用临时文件替换原始数据文件
    try {
        // 在替换之前，确保临时文件存在（如果原始文件不存在或为空且WHERE不匹配，tempFile 可能为空）
        // 实际上，上面的循环逻辑会确保 tempFile 包含所有（或部分有效）行，除非原始文件不存在。
        // fs::exists(tempFilePath) 是一个安全的检查。
        if (fs::exists(tempFilePath)) {
            fs::remove(dataFilePath); // 删除原始数据文件
            fs::rename(tempFilePath, dataFilePath); // 将临时文件重命名为原始文件名
        } else {
            // 这种情况比较异常，可能原始文件不存在（已经在上面处理），或者发生了其他文件系统错误
            // 如果 updatedRowCount > 0 但 tempFilePath 不存在，那是内部错误
            if (updatedRowCount > 0) {
                std::cerr << "Internal Error: Updated rows count > 0, but temporary file '" << tempFilePath << "' does not exist after processing." << std::endl;
                return false; // 内部错误，更新失败
            }
            // 如果 updatedRowCount == 0 且 tempFilePath 不存在，说明没有行被处理或匹配 WHERE。
            // 如果原始文件存在，它应该被保留。如果原始文件不存在，那它本来就不存在。
            // 这个分支在 updatedRowCount == 0 时，如果文件系统操作没有异常，意味着成功执行了0更新。
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error updating data: Filesystem error during file replacement: " << e.what() << std::endl;
        // 如果重命名失败，临时文件可能仍然存在。尝试清理它。
        try { fs::remove(tempFilePath); } catch(const fs::filesystem_error& e2) {
            std::cerr << "Error cleaning up temporary file after rename failure: " << e2.what() << std::endl;
        }
        // 原始文件可能已经被删除了。文件系统操作失败，更新失败。
        return false;
    }

    // 15. 如果有行被更新，则更新表的元数据 (最后修改时间)
    if (updatedRowCount > 0) {
        updateTableLastModifiedDate(dbName, tableName); // 更新最后修改时间 (记录数不因更新而改变)
        std::cout << "Successfully updated " << updatedRowCount << " rows in table '" << tableName << "'." << std::endl;
    } else {
        // 如果没有行符合 WHERE 条件，也没有更新发生。这不算是错误。
        std::cout << "No rows matched the WHERE clause. No updates performed in table '" << tableName << "'." << std::endl;
    }

    return true; // 更新操作成功完成 (无论是否有行被更新)
}
