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

        std::cout << "Create a table definition file:\"" << tableDefFile << "\"" << std::endl;
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
