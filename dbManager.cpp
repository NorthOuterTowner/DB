#include "dbManager.h"
#include <algorithm>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QTextEdit>

dbManager::dbManager() : db_list(nullptr) {
    loadDatabases(); // 初始化时加载现有数据库
}

// 设置 QTreeWidget 指针
void dbManager::setTreeWidget(QTreeWidget* treeWidget) {
    db_list = treeWidget;
}

void dbManager::loadDatabases() {
    QFile dbFile(QString::fromStdString(dbFilePath)); // 使用 QFile 和 QString
    if (!dbFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open the database file:" << QString::fromStdString(dbFilePath);
        return;
    }

    QTextStream in(&dbFile);
    DatabaseInfo info;

    while (!in.atEnd()) {
        QString line = in.readLine(); // 读取一行
        QStringList fields = line.split(" "); // 按空格分割
        if (fields.size() >= 5) {
            info.database_id = fields[0].toInt();
            info.database_name = fields[1].toStdString();
            info.creation_date = fields[2].toStdString();
            info.character_set = fields[3].toStdString();
            info.collation = fields[4].toStdString();
            databases.push_back(info); // 存储数据库信息

            // 如果 db_list 指针不为空，根据数据库名称添加新的 QTreeWidgetItem
            if (db_list) {
                QTreeWidgetItem* newItem = new QTreeWidgetItem(db_list);
                newItem->setText(0, QString::fromStdString(info.database_name));
            }
        }
    }

    dbFile.close(); // 关闭文件
    qDebug() << "Loaded databases from file:" << QString::fromStdString(dbFilePath);

    if(db_list) {
        loadTableDescriptions(); // 初始化时加载表描述信息
    }
}

void dbManager::saveDatabases() {
    QFile dbFile(QString::fromStdString(dbFilePath)); // 使用 QFile 和 QString
    if (!dbFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Could not open the database file:" << QString::fromStdString(dbFilePath);
        return;
    }

    QTextStream out(&dbFile);
    for (const auto& db : databases) {
        out << db.database_id << " "
            << QString::fromStdString(db.database_name) << " "
            << QString::fromStdString(db.creation_date) << " "
            << QString::fromStdString(db.character_set) << " "
            << QString::fromStdString(db.collation) << "\n"; // 每个数据库信息写入新行
    }

    dbFile.close(); // 关闭文件
    qDebug() << "Database information saved to:" << QString::fromStdString(dbFilePath);
}

bool dbManager::createDatabase(const std::string& name) {
    // 检查数据库是否已经存在
    auto it = std::find_if(databases.begin(), databases.end(), [&](const DatabaseInfo& db) {
        return db.database_name == name;
    });

    if (it != databases.end()) {
        qWarning() << "Database" << QString::fromStdString(name) << "already exists.";
        return false;
    }

    DatabaseInfo newDB;
    newDB.database_id = databases.size() + 1; // 自增长的ID
    newDB.database_name = name;

    // 获取当前时间作为创建日期
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%H:%M:%S"); // 格式化时间
    newDB.creation_date = oss.str(); // 字符串格式的当前时间

    // 确保字符集和排序规则正确赋值
    newDB.character_set = "utf8"; // 默认字符集
    newDB.collation = "utf8_general_ci"; // 默认排序规则

    databases.push_back(newDB);

    // 如果 db_list 指针不为空，添加新的 QTreeWidgetItem
    if (db_list) {
        QTreeWidgetItem* newItem = new QTreeWidgetItem(db_list);
        newItem->setText(0, QString::fromStdString(name));
    }

    saveDatabases(); // 保存到文件
    return true;
}

bool dbManager::dropDatabase(const std::string& name) {
    auto it = std::remove_if(databases.begin(), databases.end(), [&](const DatabaseInfo& db) {
        return db.database_name == name;
    });

    if (it != databases.end()) {
        databases.erase(it, databases.end()); // 删除找到的数据库
        qDebug() << "Database" << QString::fromStdString(name) << "dropped successfully.";
        saveDatabases(); // 保存到文件

        // 找到并删除对应的表描述文件
        std::string tableDescFile = "../../res/" + name + ".tb.txt";
        QFile tbFile(QString::fromStdString(tableDescFile));
        if (tbFile.exists()) {
            if (tbFile.remove()) {
                qDebug() << "Table description file" << QString::fromStdString(tableDescFile) << "deleted successfully.";
            } else {
                qWarning() << "Failed to delete table description file:" << QString::fromStdString(tableDescFile);
            }
        }

        // 如果 db_list 指针不为空，移除对应的 QTreeWidgetItem
        if (db_list) {
            for (int i = 0; i < db_list->topLevelItemCount(); ++i) {
                QTreeWidgetItem* item = db_list->topLevelItem(i);
                if (item->text(0) == QString::fromStdString(name)) {
                    delete item;
                    break;
                }
            }
        }

        return true;
    }

    qWarning() << "Database" << QString::fromStdString(name) << "does not exist.";
    return false;
}

std::vector<std::string> dbManager::getDatabaseNames() {
    std::vector<std::string> names;
    for (const auto& db : databases) {
        names.push_back(db.database_name);
    }
    return names;
}

void dbManager::reloadDatabases() {
    databases.clear(); // 清空现有数据库信息
    loadDatabases();
}

void dbManager::setCurrentDatabase(const std::string& dbName) {
    currentDatabase = dbName;
    qDebug() << "Current database set to: " << QString::fromStdString(currentDatabase);
}

void dbManager::addTableToDatabase(const std::string& dbName, const std::string& tableName) {
    databaseTables[dbName].push_back(tableName);
    if (db_list) {
        for (int i = 0; i < db_list->topLevelItemCount(); ++i) {
            QTreeWidgetItem* databaseItem = db_list->topLevelItem(i);
            if (databaseItem->text(0) == QString::fromStdString(dbName)) {
                QTreeWidgetItem* newTableItem = new QTreeWidgetItem(databaseItem);
                newTableItem->setText(0, QString::fromStdString(tableName));
                break;
            }
        }
    }
}

void dbManager::dropTableFromDatabase(const std::string& dbName, const std::string& tableName) {
    auto it = databaseTables.find(dbName);
    if (it != databaseTables.end()) {
        auto tableIt = std::find(it->second.begin(), it->second.end(), tableName);
        if (tableIt != it->second.end()) {
            it->second.erase(tableIt);
            qDebug() << "Table " << QString::fromStdString(tableName) << " dropped from database " << QString::fromStdString(dbName);
        }
    }

    // 如果 db_list 指针不为空，移除对应的 QTreeWidgetItem
    if (db_list) {
        for (int i = 0; i < db_list->topLevelItemCount(); ++i) {
            QTreeWidgetItem* databaseItem = db_list->topLevelItem(i);
            if (databaseItem->text(0) == QString::fromStdString(dbName)) {
                for (int j = 0; j < databaseItem->childCount(); ++j) {
                    QTreeWidgetItem* tableItem = databaseItem->child(j);
                    if (tableItem->text(0) == QString::fromStdString(tableName)) {
                        delete tableItem;
                        break;
                    }
                }
                break;
            }
        }
    }
}

std::vector<std::string> dbManager::getDatabaseTables(const std::string& dbName) {
    auto it = databaseTables.find(dbName);
    if (it != databaseTables.end()) {
        return it->second;
    }
    return {};
}

void dbManager::loadTableDescriptions() {
    if (!db_list) {
        return;
    }

    for (const auto& db : databases) {
        std::string tableDescFile = "../../res/" + db.database_name + ".tb.txt";
        QFile tbFile(QString::fromStdString(tableDescFile));
        if (tbFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&tbFile);
            QString line;
            while ((line = in.readLine()) != "") {
                std::vector<std::string> parts = split(line.toStdString(), " ");
                std::string tableName = parts[0];

                // 在 QTreeWidget 中找到对应的数据库项
                for (int i = 0; i < db_list->topLevelItemCount(); ++i) {
                    QTreeWidgetItem* databaseItem = db_list->topLevelItem(i);
                    if (databaseItem->text(0) == QString::fromStdString(db.database_name)) {
                        QTreeWidgetItem* newTableItem = new QTreeWidgetItem(databaseItem);
                        newTableItem->setText(0, QString::fromStdString(tableName));
                        break;
                    }
                }
            }
            tbFile.close();
        } else {
            qWarning() << "Could not open the table description file:" << QString::fromStdString(tableDescFile);
        }
    }
}

std::vector<std::string> dbManager::split(const std::string& s, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0, end = 0;
    while ((end = s.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + delimiter.length();
    }
    tokens.push_back(s.substr(start));
    return tokens;
}
