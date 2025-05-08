#ifndef AFFAIR_H
#define AFFAIR_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <map>
#include <vector>
#include <string>
#include <regex>
#include "utils.h"

class Lexer;

class Affair {
public:
    Affair(const QString& filePath, Lexer* lexer);
    ~Affair();

    void start();
    void commit();
    void rollback();
    void writeToUndo(const QString& sql);

    bool isrunning=false;

    QString undoSQL;

private:
    QString filePath; // 撤销日志文件路径
    QFile file;       // 文件对象
    QTextStream out;  // 文件输出流
    Lexer* lexer;     // Lexer 对象的指针


    bool end=false;


    void writeUndo(const QString& sql);
    std::map<std::string, std::string> parseSQL(const std::string& sql);
};

#endif // AFFAIR_H
