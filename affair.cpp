#include "affair.h"
#include "session.h"
#include "logger.h"
#include <QDebug>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <regex>

Affair::Affair(const QString& filePath) : filePath(filePath) {
    // 打开文件以写模式，覆盖旧文件
    file.setFileName(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件：" << filePath;
    } else {
        qDebug() << "已打开文件 undo.txt";
    }
    out.setDevice(&file);
}

Affair::~Affair() {
    // 关闭文件
    if (file.isOpen()) {
        file.close();
    }
}

void Affair::start() {
    // 初始化事务，创建或覆盖 undo.txt 文件
    file.resize(0); // 清空文件内容

    // 创建 Logger 实例并记录日志
    Logger logger("../../res/system_logs.txt"); // 指定日志文件路径
    logger.log(Session::getCurrentUserId(), "START", "TRANSACTION", filePath.toStdString());

    qDebug() << "事务开始，初始化撤销日志文件：" << filePath;
}

void Affair::commit() {
    // 提交事务

    // 创建 Logger 实例并记录日志
    Logger logger("../../res/system_logs.txt"); // 指定日志文件路径
    logger.log(Session::getCurrentUserId(), "COMMIT", "TRANSACTION", filePath.toStdString());

    qDebug() << "事务提交，撤销日志文件：" << filePath;
}

void Affair::rollback() {
    // 回滚事务

    // 创建 Logger 实例并记录日志
    Logger logger("../../res/system_logs.txt"); // 指定日志文件路径
    logger.log(Session::getCurrentUserId(), "ROLLBACK", "TRANSACTION", filePath.toStdString());

    qDebug() << "事务回滚，撤销日志文件：" << filePath;
}

void Affair::writeToUndo(const QString& sql) {
    qDebug() << "事务读取sql语句：" << sql;
    std::map<std::string, std::string> operation = parseSQL(sql.toStdString());
    QString undoSQL;

    // 根据操作类型生成反操作 SQL 语句
    if (operation["type"] == "INSERT") {
        // 插入操作的反操作是删除
        QString table = operation["table"].c_str();
        QString whereClause;
        for (const auto& [column, value] : operation) {
            if (column != "type" && column != "table") {
                whereClause += QString("%1 = '%2' AND ").arg(column.c_str()).arg(value.c_str());
            }
        }
        whereClause.chop(5); // 去掉多余的 " AND "
        undoSQL = QString("DELETE FROM %1 WHERE %2;").arg(table).arg(whereClause);
    } else if (operation["type"] == "UPDATE") {
        // 更新操作的反操作是将值更新回旧值
        QString table = operation["table"].c_str();
        QString setClause;
        QString whereClause;
        for (const auto& [column, value] : operation) {
            if (column != "type" && column != "table") {
                if (column.find("old_") == 0) {
                    setClause += QString("%1 = '%2', ").arg(column.substr(4).c_str()).arg(value.c_str());
                } else {
                    whereClause += QString("%1 = '%2' AND ").arg(column.c_str()).arg(value.c_str());
                }
            }
        }
        setClause.chop(2); // 去掉多余的 ", "
        whereClause.chop(5); // 去掉多余的 " AND "
        undoSQL = QString("UPDATE %1 SET %2 WHERE %3;").arg(table).arg(setClause).arg(whereClause);
    } else if (operation["type"] == "DELETE") {
        // 删除操作的反操作是插入
        QString table = operation["table"].c_str();
        QString columns;
        QString values;
        for (const auto& [column, value] : operation) {
            if (column != "type" && column != "table") {
                columns += QString("%1, ").arg(column.c_str());
                values += QString("'%1', ").arg(value.c_str());
            }
        }
        columns.chop(2); // 去掉多余的 ", "
        values.chop(2); // 去掉多余的 ", "
        undoSQL = QString("INSERT INTO %1 (%2) VALUES (%3);").arg(table).arg(columns).arg(values);
    }

    qDebug() << "反操作语句：" << undoSQL;
    // 写入反操作 SQL 语句到文件
    writeUndo(undoSQL);
}

void Affair::writeUndo(const QString& sql) {
    out << sql << "\n";
}

std::map<std::string, std::string> Affair::parseSQL(const std::string& sql) {
    std::map<std::string, std::string> result;
    std::regex insertPattern(R"(INSERT\s+INTO\s+(\w+)\s*(?:\(([^)]+)\))?\s*VALUES\s*\(([^)]+)\))", std::regex_constants::icase);
    std::regex updatePattern(R"(UPDATE\s+(\w+)\s+SET\s+([^WHERE]+)\s+WHERE\s+([^;]+))", std::regex_constants::icase);
    std::regex deletePattern(R"(DELETE\s+FROM\s+(\w+)\s+WHERE\s+([^;]+))", std::regex_constants::icase);

    std::smatch match;
    if (std::regex_search(sql, match, insertPattern)) {
        result["type"] = "INSERT";
        result["table"] = match[1].str();
        std::string columnsStr = match[2].str();
        std::string valuesStr = match[3].str();

        std::vector<std::string> columns = utils::split(columnsStr, ",");
        std::vector<std::string> values = utils::split(valuesStr, ",");

        for (size_t i = 0; i < columns.size(); ++i) {
            std::string columnName = utils::strip(columns[i]);
            std::string value = utils::strip(values[i]);
            result[columnName] = value;
        }
    } else if (std::regex_search(sql, match, updatePattern)) {
        result["type"] = "UPDATE";
        result["table"] = match[1].str();
        std::string setClause = match[2].str();
        std::string whereClause = match[3].str();

        std::vector<std::string> setItems = utils::split(setClause, ",");
        for (const auto& item : setItems) {
            std::vector<std::string> keyValue = utils::split(item, "=");
            if (keyValue.size() == 2) {
                std::string columnName = utils::strip(keyValue[0]);
                std::string value = utils::strip(keyValue[1]);
                result[columnName] = value;
            }
        }

        std::vector<std::string> whereItems = utils::split(whereClause, "AND");
        for (const auto& item : whereItems) {
            std::vector<std::string> condition = utils::split(item, "=");
            if (condition.size() == 2) {
                std::string conditionColumn = utils::strip(condition[0]);
                std::string conditionValue = utils::strip(condition[1]);
                result[conditionColumn] = conditionValue;
            }
        }
    } else if (std::regex_search(sql, match, deletePattern)) {
        result["type"] = "DELETE";
        result["table"] = match[1].str();
        std::string whereClause = match[2].str();

        std::vector<std::string> whereItems = utils::split(whereClause, "AND");
        for (const auto& item : whereItems) {
            std::vector<std::string> condition = utils::split(item, "=");
            if (condition.size() == 2) {
                std::string conditionColumn = utils::strip(condition[0]);
                std::string conditionValue = utils::strip(condition[1]);
                result[conditionColumn] = conditionValue;
            }
        }
    }

    return result;
}

// 在 Affair 类中添加恢复方法
void Affair::recover() {
    QFile undoFile(filePath);
    if (undoFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&undoFile);
        while (!in.atEnd()) {
            QString undoSQL = in.readLine();
            // 执行反操作 SQL 语句
            // 这里需要调用相应的数据库操作方法来执行 undoSQL
            // 例如：datamanager->executeSQL(undoSQL);
            qDebug() << "执行反操作语句：" << undoSQL;
        }
        undoFile.close();
    }
}
