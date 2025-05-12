#include "tablemanage.h"
#include "datamanager.h"
#include "logger.h"
#include "session.h"
#include <iostream>

namespace fs = std::filesystem;

// 构造函数实现
tableManage::tableManage() : affair("undo.txt",nullptr) {
    // 构造函数初始化 affair
}

// 创建表函数实现
bool tableManage::createTable(const std::string& tableName,  const std::string& dbName) {

    // 记录事务操作
    QString sql = QString("CREATE TABLE %1.%2;").arg(QString::fromStdString(dbName)).arg(QString::fromStdString(tableName));
    affair.writeToUndo(sql);

    if (!isValidTableName(dbName, tableName)) {
        std::cerr << "Table '" << tableName << "' already exists in database '" << dbName << "'." << std::endl;
        return false;
    }

    // 日志记录表创建操作，包含数据库名
    Logger logger("../../res/system_logs.txt"); // 指定日志文件路径
    logger.log(Session::getCurrentUserId(), "CREATE", "TABLE", tableName + " in " + dbName); // 记录日志

    databaseName = dbName;

    TableInfo newTable;
    newTable.table_name = tableName;
    newTable.databaseName = databaseName;

    // 获取当前时间作为创建日期
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%H:%M:%S"); // 格式化时间
    newTable.creation_date = oss.str(); // 字符串格式的当前时间
    newTable.last_modified_date = oss.str();// 把最后修改时间初始为创建时间

    tableDescFile = "../../res/" + newTable.databaseName + ".tb.txt";

    QFile tbFile(QString::fromStdString(tableDescFile)); // 使用 QFile 和 QString
    // 修改为追加写入模式
    if (!tbFile.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Could not open the Table-Describe file:" << QString::fromStdString(tableDescFile);
        return false;
    }

    QTextStream out(&tbFile);

    out << QString::fromStdString(newTable.table_name) << " "
        << QString::fromStdString(newTable.databaseName) << " "
        << QString::fromStdString(newTable.creation_date) << " "
        << QString::fromStdString(newTable.last_modified_date) << " "
        << newTable.field_count << " "
        << newTable.record_count << " "
        << QString::fromStdString(newTable.table_type) << "\n";

    tbFile.close();
    std::cout << "Create or update a table profile:\"" << tableDescFile << "\"" << std::endl;
    return true;
}

// 字符串分割函数实现
std::vector<std::string> tableManage::split(const std::string& s, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0, end = 0;
    while ((end = s.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + delimiter.length();
    }
    tokens.push_back(s.substr(start));
    return tokens;
}

bool tableManage::isValidTableName(const std::string& dbName, const std::string& tableName) {
    std::string tableDescFile = "../../res/" + dbName + ".tb.txt";
    QFile tbFile(QString::fromStdString(tableDescFile));
    if (tbFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&tbFile);
        QString line;
        while ((line = in.readLine()) != "") {
            std::vector<std::string> parts = split(line.toStdString(), " ");
            if (parts.size() > 1 && parts[0] == tableName) {
                tbFile.close();
                return false;
            }
        }
        tbFile.close();
    } else {
        // 如果文件打不开，假设可以创建表
        qWarning() << "Could not open the table description file:" << QString::fromStdString(tableDescFile);
    }
    return true;
}

bool tableManage::alterTable(const std::string& dbName, const std::string& tableName, const std::string& operation,
                             const std::vector<std::string>& fieldNames, const std::vector<int>& fieldOrders,
                             const std::vector<std::string>& fieldTypes, const std::vector<int>& fieldTypeParams,
                             const std::vector<std::string>& constraints)
{
    QString sql;
    if (operation == "ADD") {
        sql = QString("ALTER TABLE %1.%2 ADD ...;").arg(QString::fromStdString(dbName)).arg(QString::fromStdString(tableName));
    } else if (operation == "MODIFY") {
        sql = QString("ALTER TABLE %1.%2 MODIFY ...;").arg(QString::fromStdString(dbName)).arg(QString::fromStdString(tableName));
    } else if (operation == "DROP") {
        sql = QString("ALTER TABLE %1.%2 DROP ...;").arg(QString::fromStdString(dbName)).arg(QString::fromStdString(tableName));
    }
    affair.writeToUndo(sql);


    TableInfo currentTableInfo;
    tableDescFile = "../../res/" + dbName + ".tb.txt";
    QFile tbFile(QString::fromStdString(tableDescFile));
    if (tbFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&tbFile);
        QString line = in.readLine();
        std::vector<std::string> parts = split(line.toStdString(), " ");
        currentTableInfo.table_name = parts[0];
        currentTableInfo.databaseName = parts[1];
        currentTableInfo.creation_date = parts[2];
        currentTableInfo.last_modified_date = parts[3];
        currentTableInfo.field_count = std::stoi(parts[4]);
        currentTableInfo.record_count = std::stoi(parts[5]);
        currentTableInfo.table_type = parts[6];
        tbFile.close();
    } else {
        std::cerr << "Failed to open table profile:\"" << tableDescFile << std::endl;
        return false;
    }

    fieldManage fm;
    if (operation == "ADD") {
        // 原有的 ADD 操作逻辑
        currentTableInfo.field_count += fieldNames.size();
        for (size_t i = 0; i < fieldNames.size(); ++i) {
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%H:%M:%S");
            std::string modificationTime = oss.str();

            if (!fm.addField(dbName, tableName, fieldNames[i], fieldOrders[i], fieldTypes[i], fieldTypeParams[i], modificationTime, constraints[i])) {
                std::cerr << "Failed to add field: " << fieldNames[i] << std::endl;
                return false;
            }
        }
    } else if (operation == "MODIFY") {
        // 新增的 MODIFY 操作逻辑
        for (size_t i = 0; i < fieldNames.size(); ++i) {
            if (!fm.modifyField(dbName, tableName, fieldNames[i], fieldOrders[i], fieldTypes[i], fieldTypeParams[i], constraints[i])) {
                std::cerr << "Failed to modify field: " << fieldNames[i] << std::endl;
                return false;
            }
        }
        // 更新表描述文件中的修改时间
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%H:%M:%S");
        currentTableInfo.last_modified_date = oss.str();
    } else if (operation == "DROP") {
        // 原有的 DROP 操作逻辑
        currentTableInfo.field_count -= fieldNames.size();
        for (const auto& fieldName : fieldNames) {
            if (!fm.dropField(dbName, tableName, fieldName)) {
                std::cerr << "Failed to drop field: " << fieldName << std::endl;
                return false;
            }
        }
    } else {
        std::cerr << "Invalid operation type: " << operation << std::endl;
        return false;
    }

    if (!tbFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Could not open the Table-Describe file:" << QString::fromStdString(tableDescFile);
        return false;
    }

    QTextStream out(&tbFile);

    out << QString::fromStdString(currentTableInfo.table_name) << " "
        << QString::fromStdString(currentTableInfo.databaseName) << " "
        << QString::fromStdString(currentTableInfo.creation_date) << " "
        << QString::fromStdString(currentTableInfo.last_modified_date) << " "
        << currentTableInfo.field_count << " "
        << currentTableInfo.record_count << " "
        << QString::fromStdString(currentTableInfo.table_type) << "\n";

    tbFile.close();
    return true;
}

bool tableManage::dropTable(const std::string& dbName, const std::string& tableName) {

    // 记录事务操作
    QString sql = QString("DROP TABLE %1.%2;").arg(QString::fromStdString(dbName)).arg(QString::fromStdString(tableName));
    affair.writeToUndo(sql);

    // 日志记录表删除操作，包含数据库名
    Logger logger("../../res/system_logs.txt"); // 指定日志文件路径
    logger.log(Session::getCurrentUserId(), "DROP", "TABLE", tableName + " from " + dbName); // 记录日志


    // 查找表信息文件
    tableDescFile = "../../res/" + dbName + ".tb.txt";
    QFile tbFile(QString::fromStdString(tableDescFile));
    if (!tbFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "Failed to open table profile:\"" << tableDescFile << std::endl;
        return false;
    }

    // 读取所有表信息
    QStringList allLines;
    QTextStream in(&tbFile);
    while (!in.atEnd()) {
        allLines.append(in.readLine());
    }
    tbFile.close();

    // 找到要删除的表信息并移除
    bool found = false;
    for (int i = 0; i < allLines.size(); ++i) {
        std::vector<std::string> parts = split(allLines[i].toStdString(), " ");
        if (parts[0] == tableName) {
            allLines.removeAt(i);
            found = true;
            break;
        }
    }

    if (!found) {
        std::cerr << "Table '" << tableName << "' not found in database '" << dbName << "'." << std::endl;
        return false;
    }

    // 更新表描述文件
    if (!tbFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::cerr << "Failed to open table profile for writing:\"" << tableDescFile << std::endl;
        return false;
    }
    QTextStream out(&tbFile);
    for (const auto& line : allLines) {
        out << line << "\n";
    }
    tbFile.close();

    std::string tableDefFile = "../../res/" + tableName + ".tdf.txt";
    try {
        fs::remove(tableDefFile);
        std::cout << "Deleted table definition file: " << tableDefFile << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to delete table definition file: " << e.what() << std::endl;
    }

    return true;
}


tableManage::TableInfo tableManage::getTableInfo(const std::string& dbName,const std::string& tableName){

    // 日志记录获取表信息操作，包含数据库名
    //Logger logger("../../res/system_logs.txt"); // 指定日志文件路径
    //logger.log(Session::getCurrentUserId(), "GET", "TABLE", tableName + " from " + dbName); // 记录日志

    TableInfo tableInfo;
    // 构建表描述文件路径
    std::string tableDescFile = "../../res/" + dbName + ".tb.txt";

    // 打开表描述文件
    std::ifstream file(tableDescFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open table description file: " << tableDescFile << std::endl;
        return tableInfo;
    }

    std::string line;
    while (std::getline(file, line)) {
        // 分割每行内容
        std::vector<std::string> parts = split(line, " ");
        if (parts.size() >= 7 && parts[0] == tableName) {
            // 解析表信息
            tableInfo.table_name = parts[0];
            tableInfo.databaseName = parts[1];
            tableInfo.creation_date = parts[2];
            tableInfo.last_modified_date = parts[3];
            tableInfo.field_count = std::stoi(parts[4]);
            tableInfo.record_count = std::stoi(parts[5]);
            tableInfo.table_type = parts[6];
            break;
        }
    }

    // 关闭文件
    file.close();

    return tableInfo;
}

// 在 tableManage 类中添加备份表的函数
void tableManage::backupTable(const std::string& dbName, const std::string& tableName) {
    std::string tableDefFile = "../../res/" + tableName + ".tdf.txt";
    std::string backupTableDefFile = "../../res/backup/" + tableName + "_tdf_backup.txt";
    QFile defFile(QString::fromStdString(tableDefFile));
    QFile backupDefFile(QString::fromStdString(backupTableDefFile));

    if (defFile.open(QIODevice::ReadOnly | QIODevice::Text) && backupDefFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream in(&defFile);
        QTextStream out(&backupDefFile);
        out << in.readAll();
        defFile.close();
        backupDefFile.close();
    }

    datamanager dataMgr(nullptr);
    std::string dataFilePath = dataMgr.buildFilePath(dbName, tableName);
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

//更新表的记录数
void tableManage::updateTableRecordCount(const std::string& dbName,const std::string& tableName,int delta){
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
            // 更新记录数
            int recordCount = std::stoi(parts[2]); // 假设记录数在第 3 列
            recordCount += delta;
            parts[2] = std::to_string(recordCount);

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

std::vector<tableManage::TableInfo> tableManage::getTablesInDatabase(const std::string& dbName) {
    std::vector<TableInfo> tables; // 存储表信息的向量
    std::string tableDescFile = "../../res/" + dbName + ".tb.txt"; // 构建表描述文件路径

    QFile tbFile(QString::fromStdString(tableDescFile));
    if (tbFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&tbFile);
        QString line;
        // 逐行读取文件
        while (in.readLineInto(&line)) {
            std::vector<std::string> parts = split(line.toStdString(), " ");
            if (parts.size() >= 7) { // 确保行内容足够表示一个表
                TableInfo info;
                info.table_name = parts[0];
                info.databaseName = parts[1];
                info.creation_date = parts[2];
                info.last_modified_date = parts[3];
                info.field_count = std::stoi(parts[4]);
                info.record_count = std::stoi(parts[5]);
                info.table_type = parts[6];
                tables.push_back(info); // 将表信息添加到向量中
            }
        }
        tbFile.close(); // 关闭文件
    } else {
        std::cerr << "Could not open table description file: " << tableDescFile << std::endl;
    }

    return tables; // 返回所有找到的表的信息
}
