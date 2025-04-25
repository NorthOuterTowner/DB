#include "fieldmanage.h"
#include <chrono>
#include <iomanip>
#include <sstream>

fieldManage::fieldManage() {}

bool fieldManage::addField(const std::string& dbName, const std::string& tableName, const std::string& fieldName, int fieldOrder, const std::string& fieldType, int fieldTypeParams, const std::string& modificationTime, const std::string& constraints) {
    FieldInfo info(dbName, tableName, fieldName, fieldOrder, fieldType, fieldTypeParams, modificationTime, constraints);

    if (!isFieldNameValid(info.fieldName)) {
        std::cerr << "Invalid field name: " << info.fieldName << std::endl;
        return false;
    }

    if (!updateTableDescAndDefFiles(info, "ADD")) {
        return false;
    }

    std::cout << "Create or update a table definition file:\"" << getTableDefFilePath(tableName) << "\"" << std::endl;
    return true;
}

std::string fieldManage::getTableDescFilePath(const std::string& dbName) {
    return "../../res/" + dbName + ".tb.txt";
}

std::string fieldManage::getTableDefFilePath(const std::string& tableName) {
    return "../../res/" + tableName + ".tdf.txt";
}

bool fieldManage::isFieldNameValid(const std::string& fieldName) {
    return fieldName.length() <= 128;
}

bool fieldManage::updateTableDescAndDefFiles(const FieldInfo& info, const std::string& operation) {
    if (!updateTableDescFile(info, operation)) {
        return false;
    }

    if (!createOrUpdateTableDefFile(info, operation)) {
        return false;
    }

    return true;
}

bool fieldManage::updateTableDescFile(const FieldInfo& info, const std::string& operation) {
    std::string tableDescFile = getTableDescFilePath(info.dbName);
    QFile tbFile(QString::fromStdString(tableDescFile));

    if (!tbFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "Failed to open table description file: " << tableDescFile << std::endl;
        return false;
    }

    QStringList allLines;
    QTextStream in(&tbFile);
    while (!in.atEnd()) {
        allLines.append(in.readLine());
    }
    tbFile.close();

    bool found = false;
    for (int i = 0; i < allLines.size(); ++i) {
        std::vector<std::string> parts = split(allLines[i].toStdString(), " ");
        if (parts.size() > 1 && parts[0] == info.tableName) {
            found = true;
            // 更新修改时间为当前时间
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%H:%M:%S");
            std::string modificationTime = oss.str();
            parts[3] = modificationTime;

            if (operation == "ADD") {
                // 字段数自增一
                int fieldCount = std::stoi(parts[4]);
                parts[4] = std::to_string(fieldCount + 1);

                parts.push_back(info.fieldName);
                parts.push_back(std::to_string(info.fieldOrder));
                parts.push_back(info.fieldType);
                parts.push_back(std::to_string(info.fieldTypeParams));
                parts.push_back(info.constraints);
            } else if (operation == "DROP") {
                bool fieldFound = false;
                // 移除字段相关信息
                for (size_t j = 5; j < parts.size(); j += 5) {
                    if (parts[j] == info.fieldName) {
                        fieldFound = true;
                        parts.erase(parts.begin() + j, parts.begin() + j + 5);
                        break;
                    }
                }
                if (fieldFound) {
                    // 更新字段数
                    int fieldCount = std::stoi(parts[4]);
                    parts[4] = std::to_string(fieldCount - 1);
                }
            }

            QString newLine;
            for (const auto& part : parts) {
                newLine += QString::fromStdString(part) + " ";
            }
            allLines[i] = newLine.trimmed();
            break;
        }
    }

    if (!found) {
        std::cerr << "Table " << info.tableName << " not found in database " << info.dbName << std::endl;
        return false;
    }

    if (!tbFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::cerr << "Failed to open table description file for writing: " << tableDescFile << std::endl;
        return false;
    }

    QTextStream out(&tbFile);
    for (const auto& line : allLines) {
        out << line << "\n";
    }
    tbFile.close();

    return true;
}

bool fieldManage::createOrUpdateTableDefFile(const FieldInfo& info, const std::string& operation) {
    std::string tableDefFile = getTableDefFilePath(info.tableName);
    QFile defFile(QString::fromStdString(tableDefFile));

    if (operation == "ADD") {
        if (!defFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
            std::cerr << "Failed to open table definition file: " << tableDefFile << std::endl;
            return false;
        }

        QTextStream defOut(&defFile);
        defOut << QString::fromStdString(info.tableName) << " ";
        defOut << QString::fromStdString(info.dbName) << " ";
        defOut << QString::fromStdString(info.fieldName) << " ";
        defOut << info.fieldOrder << " ";
        defOut << QString::fromStdString(info.fieldType) << " ";
        defOut << info.fieldTypeParams << " ";
        defOut << QString::fromStdString(info.modificationTime) << " ";
        defOut << QString::fromStdString(info.constraints) << "\n";
        defFile.close();

        std::cout << "Create a table definition file:\"" << tableDefFile << std::endl;
    } else if (operation == "DROP") {
        if (!defFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            std::cerr << "Failed to open table definition file: " << tableDefFile << std::endl;
            return false;
        }

        QStringList defLines;
        QTextStream defIn(&defFile);
        while (!defIn.atEnd()) {
            defLines.append(defIn.readLine());
        }
        defFile.close();

        bool defFieldFound = false;
        for (int i = 0; i < defLines.size(); ++i) {
            std::vector<std::string> parts = split(defLines[i].toStdString(), " ");
            if (parts.size() > 2 && parts[2] == info.fieldName) {
                defFieldFound = true;
                defLines.removeAt(i);
                break;
            }
        }

        if (!defFieldFound) {
            std::cerr << "Field " << info.fieldName << " not found in table definition file." << std::endl;
            return false;
        }

        if (!defFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            std::cerr << "Failed to open table definition file for writing: " << tableDefFile << std::endl;
            return false;
        }

        QTextStream defOut(&defFile);
        for (const auto& line : defLines) {
            defOut << line << "\n";
        }
        defFile.close();
    }

    return true;
}

std::vector<std::string> fieldManage::split(const std::string& s, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0, end = 0;
    while ((end = s.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + delimiter.length();
    }
    tokens.push_back(s.substr(start));
    return tokens;
}

// 新增删除字段的方法
bool fieldManage::dropField(const std::string& dbName, const std::string& tableName, const std::string& fieldName) {
    FieldInfo info(dbName, tableName, fieldName, 0, "", 0, "", "");
    return updateTableDescAndDefFiles(info, "DROP");
}

// 新增修改字段的方法
bool fieldManage::modifyField(const std::string& dbName, const std::string& tableName, const std::string& fieldName, int fieldOrder, const std::string& fieldType, int fieldTypeParams, const std::string& constraints) {
    std::string tableDefFile = getTableDefFilePath(tableName);
    QFile defFile(QString::fromStdString(tableDefFile));
    if (!defFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "Failed to open table definition file: " << tableDefFile << std::endl;
        return false;
    }

    QStringList defLines;
    QTextStream defIn(&defFile);
    while (!defIn.atEnd()) {
        defLines.append(defIn.readLine());
    }
    defFile.close();

    bool fieldFound = false;
    for (int i = 0; i < defLines.size(); ++i) {
        std::vector<std::string> parts = split(defLines[i].toStdString(), " ");
        if (parts.size() > 2 && parts[2] == fieldName) {
            fieldFound = true;
            parts[3] = std::to_string(fieldOrder);
            parts[4] = fieldType;
            parts[5] = std::to_string(fieldTypeParams);
            //parts[7] = constraints;

            QString newLine;
            for (const auto& part : parts) {
                newLine += QString::fromStdString(part) + " ";
            }
            defLines[i] = newLine.trimmed();
            break;
        }
    }

    if (!fieldFound) {
        std::cerr << "Field " << fieldName << " not found in table definition file." << std::endl;
        return false;
    }

    if (!defFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::cerr << "Failed to open table definition file for writing: " << tableDefFile << std::endl;
        return false;
    }

    QTextStream defOut(&defFile);
    for (const auto& line : defLines) {
        defOut << line << "\n";
    }
    defFile.close();

    // 更新表描述文件
    std::string tableDescFile = getTableDescFilePath(dbName);
    QFile tbFile(QString::fromStdString(tableDescFile));

    if (!tbFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "Failed to open table description file: " << tableDescFile << std::endl;
        return false;
    }

    QStringList allLines;
    QTextStream in(&tbFile);
    while (!in.atEnd()) {
        allLines.append(in.readLine());
    }
    tbFile.close();

    bool found = false;
    for (int i = 0; i < allLines.size(); ++i) {
        std::vector<std::string> parts = split(allLines[i].toStdString(), " ");
        if (parts.size() > 1 && parts[0] == tableName) {
            found = true;
            // 更新字段信息
            for (size_t j = 5; j < parts.size(); j += 5) {
                if (parts[j] == fieldName) {
                    parts[j + 1] = std::to_string(fieldOrder);
                    parts[j + 2] = fieldType;
                    parts[j + 3] = std::to_string(fieldTypeParams);
                    parts[j + 4] = constraints;

                    // 更新修改时间为当前时间
                    auto now = std::chrono::system_clock::now();
                    auto in_time_t = std::chrono::system_clock::to_time_t(now);
                    std::ostringstream oss;
                    oss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%H:%M:%S");
                    std::string modificationTime = oss.str();
                    parts[3] = modificationTime;
                    break;
                }
            }

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
        return false;
    }

    if (!tbFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::cerr << "Failed to open table description file for writing: " << tableDescFile << std::endl;
        return false;
    }

    QTextStream out(&tbFile);
    for (const auto& line : allLines) {
        out << line << "\n";
    }
    tbFile.close();

    std::cout << "Modified field: " << fieldName << " in table: " << tableName << std::endl;
    return true;
}

std::vector<fieldManage::FieldInfo>fieldManage::getFieldsInfo(const std::string& tableName){
    std::vector<fieldManage::FieldInfo> fields;
    std::string tableDefFile =fieldManage::getTableDefFilePath(tableName);
    QFile defFile(QString::fromStdString(tableDefFile));

    if (!defFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // 文件不存在或打开失败，对于获取字段列表来说，这意味着表没有定义字段或表不存在定义文件
        // 打印警告或错误，并返回空向量
        std::cerr << "Warning: Could not open table definition file '" << tableDefFile << "' to get fields." << std::endl;
        return fields; // 返回空向量
    }

    QTextStream defIn(&defFile);
    while (!defIn.atEnd()) {
        QString line = defIn.readLine();
        std::vector<std::string> parts = fieldManage::split(line.toStdString(), " ");

        // 假设 .tdf.txt 文件的每行是 "tableName dbName fieldName fieldOrder fieldType fieldTypeParams modificationTime constraints"
        // 需要至少有 8 部分来构成一个完整的 FieldInfo
        if (parts.size() >= 8) {
            // 尝试解析各部分并创建 FieldInfo 对象
            try {
                std::string currentTableName = parts[0];
                std::string dbName = parts[1];
                std::string fieldName = parts[2];
                int fieldOrder = std::stoi(parts[3]);
                std::string fieldType = parts[4];
                int fieldTypeParams = std::stoi(parts[5]);
                std::string modificationTime = parts[6];
                std::string constraints;
                // constraints 可能会包含空格，需要特殊处理或假设它是最后一列的剩余部分
                // 简单的处理：假设 constraints 是从 parts[7] 开始到行尾的所有内容，用空格重新连接
                for (size_t i = 7; i < parts.size(); ++i) {
                    if (i > 7) constraints += " ";
                    constraints += parts[i];
                }


                // 创建 FieldInfo 对象并添加到向量
                // 注意：FieldInfo 构造函数的参数顺序需要和 parts 的解析顺序对应
                fields.emplace_back(dbName, currentTableName, fieldName, fieldOrder, fieldType, fieldTypeParams, modificationTime, constraints);

            } catch (const std::exception& e) {
                std::cerr << "Error parsing field definition line in '" << tableDefFile << "': " << line.toStdString() << " - " << e.what() << std::endl;
                // 可以选择跳过此行或返回错误
                continue; // 跳过格式错误的行
            }
        } else {
            std::cerr << "Warning: Skipping malformed line in table definition file '" << tableDefFile << "': " << line.toStdString() << std::endl;
        }
    }
    defFile.close();

    // 可选：根据 fieldOrder 对字段进行排序，以便它们按照 CREATE TABLE 中定义的顺序排列
    std::sort(fields.begin(), fields.end(), [](const FieldInfo& a, const FieldInfo& b) {
        return a.fieldOrder < b.fieldOrder;
    });


    return fields;
}
