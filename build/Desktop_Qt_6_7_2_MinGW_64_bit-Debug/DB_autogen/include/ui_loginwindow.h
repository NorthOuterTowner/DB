/********************************************************************************
** Form generated from reading UI file 'loginwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGINWINDOW_H
#define UI_LOGINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_loginwindow
{
public:
    QWidget *centralwidget;
    QPushButton *button_signin;
    QPushButton *button_exit;
    QLineEdit *user_line;
    QLabel *user_lable;
    QLabel *title;
    QLabel *user_photo;
    QPushButton *button_register;
    QLabel *code_lable;
    QLineEdit *code_line;
    QLabel *code_photo;
    QMenuBar *menubar;
    QMenu *menu;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *loginwindow)
    {
        if (loginwindow->objectName().isEmpty())
            loginwindow->setObjectName("loginwindow");
        loginwindow->resize(800, 600);
        loginwindow->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 255);"));
        centralwidget = new QWidget(loginwindow);
        centralwidget->setObjectName("centralwidget");
        button_signin = new QPushButton(centralwidget);
        button_signin->setObjectName("button_signin");
        button_signin->setGeometry(QRect(340, 350, 91, 31));
        button_signin->setStyleSheet(QString::fromUtf8("border-color: rgb(0, 85, 255);\n"
"background-color: rgb(170, 170, 127);\n"
"color: rgb(0, 0, 0);"));
        button_exit = new QPushButton(centralwidget);
        button_exit->setObjectName("button_exit");
        button_exit->setGeometry(QRect(500, 350, 91, 31));
        button_exit->setStyleSheet(QString::fromUtf8("border-color: rgb(0, 85, 255);\n"
"background-color: rgb(170, 170, 127);\n"
"color: rgb(0, 0, 0);"));
        user_line = new QLineEdit(centralwidget);
        user_line->setObjectName("user_line");
        user_line->setGeometry(QRect(332, 239, 161, 31));
        QFont font;
        font.setFamilies({QString::fromUtf8("Titillium Web")});
        font.setPointSize(18);
        user_line->setFont(font);
        user_line->setStyleSheet(QString::fromUtf8("border-color: rgb(0, 0, 0);\n"
"color: rgb(0, 0, 0);"));
        user_lable = new QLabel(centralwidget);
        user_lable->setObjectName("user_lable");
        user_lable->setGeometry(QRect(270, 240, 71, 31));
        QFont font1;
        font1.setFamilies({QString::fromUtf8("STLiti")});
        font1.setPointSize(22);
        user_lable->setFont(font1);
        user_lable->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        title = new QLabel(centralwidget);
        title->setObjectName("title");
        title->setGeometry(QRect(250, 50, 351, 91));
        QFont font2;
        font2.setFamilies({QString::fromUtf8("\345\215\216\346\226\207\347\220\245\347\217\200")});
        font2.setPointSize(72);
        title->setFont(font2);
        title->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        user_photo = new QLabel(centralwidget);
        user_photo->setObjectName("user_photo");
        user_photo->setGeometry(QRect(220, 230, 41, 41));
        button_register = new QPushButton(centralwidget);
        button_register->setObjectName("button_register");
        button_register->setGeometry(QRect(180, 350, 91, 31));
        button_register->setStyleSheet(QString::fromUtf8("border-color: rgb(0, 85, 255);\n"
"background-color: rgb(170, 170, 127);\n"
"color: rgb(0, 0, 0);"));
        code_lable = new QLabel(centralwidget);
        code_lable->setObjectName("code_lable");
        code_lable->setGeometry(QRect(270, 290, 71, 31));
        code_lable->setFont(font1);
        code_lable->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        code_line = new QLineEdit(centralwidget);
        code_line->setObjectName("code_line");
        code_line->setGeometry(QRect(330, 290, 161, 31));
        QFont font3;
        font3.setFamilies({QString::fromUtf8("Titillium Web")});
        font3.setPointSize(11);
        code_line->setFont(font3);
        code_line->setStyleSheet(QString::fromUtf8("border-color: rgb(0, 0, 0);\n"
"color: rgb(0, 0, 0);"));
        code_photo = new QLabel(centralwidget);
        code_photo->setObjectName("code_photo");
        code_photo->setGeometry(QRect(220, 280, 41, 41));
        loginwindow->setCentralWidget(centralwidget);
        button_signin->raise();
        button_exit->raise();
        user_lable->raise();
        title->raise();
        user_photo->raise();
        button_register->raise();
        code_lable->raise();
        code_photo->raise();
        user_line->raise();
        code_line->raise();
        menubar = new QMenuBar(loginwindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 17));
        menubar->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"background-color: rgb(134, 134, 134);"));
        menu = new QMenu(menubar);
        menu->setObjectName("menu");
        loginwindow->setMenuBar(menubar);
        statusbar = new QStatusBar(loginwindow);
        statusbar->setObjectName("statusbar");
        loginwindow->setStatusBar(statusbar);

        menubar->addAction(menu->menuAction());

        retranslateUi(loginwindow);

        QMetaObject::connectSlotsByName(loginwindow);
    } // setupUi

    void retranslateUi(QMainWindow *loginwindow)
    {
        loginwindow->setWindowTitle(QCoreApplication::translate("loginwindow", "loginwindow", nullptr));
        button_signin->setText(QCoreApplication::translate("loginwindow", "\347\231\273\345\275\225", nullptr));
        button_exit->setText(QCoreApplication::translate("loginwindow", "\351\200\200\345\207\272", nullptr));
        user_line->setText(QCoreApplication::translate("loginwindow", "123213", nullptr));
        user_lable->setText(QCoreApplication::translate("loginwindow", "\347\224\250\346\210\267\357\274\232", nullptr));
        title->setText(QCoreApplication::translate("loginwindow", "DBMS", nullptr));
        user_photo->setText(QCoreApplication::translate("loginwindow", "TextLabel", nullptr));
        button_register->setText(QCoreApplication::translate("loginwindow", "\346\263\250\345\206\214", nullptr));
        code_lable->setText(QCoreApplication::translate("loginwindow", "\345\257\206\347\240\201\357\274\232", nullptr));
        code_line->setText(QCoreApplication::translate("loginwindow", "\342\200\273\342\200\273\342\200\273\342\200\273", nullptr));
        code_photo->setText(QCoreApplication::translate("loginwindow", "TextLabel", nullptr));
        menu->setTitle(QCoreApplication::translate("loginwindow", "\347\231\273\345\275\225\347\225\214\351\235\242", nullptr));
    } // retranslateUi

};

namespace Ui {
    class loginwindow: public Ui_loginwindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINWINDOW_H
