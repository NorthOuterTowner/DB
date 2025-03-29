#include "loginwindow.h"
#include "lexer.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QDir>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //显示当前工作目录
    //qDebug() << "Current working directory:" << QDir::currentPath();

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "DB_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    loginwindow w;
    w.show();
    return a.exec();
}
