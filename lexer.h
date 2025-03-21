#ifndef LEXER_H
#define LEXER_H

#include <QObject>
#include <string>

class Lexer : public QObject
{
    Q_OBJECT

public:
    Lexer();

public slots:
    void handleRawSQL(QString rawSql);
};

#endif // LEXER_H
