/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *exit;
    QAction *action_2;
    QAction *action_4;
    QAction *newdb;
    QAction *newtable;
    QAction *yunx;
    QAction *clear;
    QAction *save;
    QAction *exit_2;
    QWidget *centralwidget;
    QFrame *frame;
    QTreeWidget *db_list;
    QTextEdit *textEdit;
    QTableWidget *tableWidget_2;
    QToolBar *toolBar;
    QMenuBar *menubar;
    QMenu *new_db;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        QFont font;
        font.setPointSize(8);
        MainWindow->setFont(font);
        MainWindow->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 255);"));
        exit = new QAction(MainWindow);
        exit->setObjectName("exit");
        action_2 = new QAction(MainWindow);
        action_2->setObjectName("action_2");
        action_4 = new QAction(MainWindow);
        action_4->setObjectName("action_4");
        newdb = new QAction(MainWindow);
        newdb->setObjectName("newdb");
        newtable = new QAction(MainWindow);
        newtable->setObjectName("newtable");
        yunx = new QAction(MainWindow);
        yunx->setObjectName("yunx");
        clear = new QAction(MainWindow);
        clear->setObjectName("clear");
        save = new QAction(MainWindow);
        save->setObjectName("save");
        exit_2 = new QAction(MainWindow);
        exit_2->setObjectName("exit_2");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        frame = new QFrame(centralwidget);
        frame->setObjectName("frame");
        frame->setGeometry(QRect(0, 0, 801, 571));
        frame->setFrameShape(QFrame::Shape::StyledPanel);
        frame->setFrameShadow(QFrame::Shadow::Raised);
        db_list = new QTreeWidget(frame);
        QBrush brush(QColor(0, 0, 0, 255));
        brush.setStyle(Qt::Dense2Pattern);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setForeground(0, brush);
        db_list->setHeaderItem(__qtreewidgetitem);
        db_list->setObjectName("db_list");
        db_list->setGeometry(QRect(0, 0, 141, 471));
        db_list->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"border-color: rgb(0, 0, 0);"));
        textEdit = new QTextEdit(frame);
        textEdit->setObjectName("textEdit");
        textEdit->setGeometry(QRect(160, 20, 581, 191));
        textEdit->setStyleSheet(QString::fromUtf8("border-color: rgb(0, 0, 0);"));
        tableWidget_2 = new QTableWidget(frame);
        if (tableWidget_2->columnCount() < 4)
            tableWidget_2->setColumnCount(4);
        QBrush brush1(QColor(0, 0, 0, 255));
        brush1.setStyle(Qt::SolidPattern);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        __qtablewidgetitem->setForeground(brush1);
        tableWidget_2->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        if (tableWidget_2->rowCount() < 4)
            tableWidget_2->setRowCount(4);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tableWidget_2->setItem(0, 0, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        tableWidget_2->setItem(0, 1, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        tableWidget_2->setItem(0, 2, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        tableWidget_2->setItem(0, 3, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        tableWidget_2->setItem(1, 0, __qtablewidgetitem8);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        tableWidget_2->setItem(2, 0, __qtablewidgetitem9);
        QTableWidgetItem *__qtablewidgetitem10 = new QTableWidgetItem();
        tableWidget_2->setItem(3, 0, __qtablewidgetitem10);
        tableWidget_2->setObjectName("tableWidget_2");
        tableWidget_2->setGeometry(QRect(160, 230, 581, 241));
        tableWidget_2->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"border-color: rgb(0, 0, 0);"));
        MainWindow->setCentralWidget(centralwidget);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName("toolBar");
        toolBar->setStyleSheet(QString::fromUtf8("border-color: rgb(0, 0, 0);\n"
"background-color: rgb(195, 195, 195);\n"
"color: rgb(0, 0, 0);"));
        toolBar->setMovable(false);
        MainWindow->addToolBar(Qt::ToolBarArea::TopToolBarArea, toolBar);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 25));
        menubar->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"background-color: rgb(134, 134, 134);"));
        new_db = new QMenu(menubar);
        new_db->setObjectName("new_db");
        MainWindow->setMenuBar(menubar);

        menubar->addAction(new_db->menuAction());
        new_db->addAction(exit_2);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        exit->setText(QCoreApplication::translate("MainWindow", "\351\200\200\345\207\272", nullptr));
        action_2->setText(QCoreApplication::translate("MainWindow", "\350\277\220\350\241\214", nullptr));
        action_4->setText(QCoreApplication::translate("MainWindow", "\346\270\205\347\251\272", nullptr));
        newdb->setText(QCoreApplication::translate("MainWindow", "\346\226\260\345\273\272\346\225\260\346\215\256\345\272\223", nullptr));
        newtable->setText(QCoreApplication::translate("MainWindow", "\346\226\260\345\273\272\350\241\250", nullptr));
        yunx->setText(QCoreApplication::translate("MainWindow", "\350\277\220\350\241\214", nullptr));
        clear->setText(QCoreApplication::translate("MainWindow", "\346\270\205\347\251\272", nullptr));
        save->setText(QCoreApplication::translate("MainWindow", "\344\277\235\345\255\230", nullptr));
        exit_2->setText(QCoreApplication::translate("MainWindow", "\351\200\200\345\207\272\347\231\273\345\275\225", nullptr));
        QTreeWidgetItem *___qtreewidgetitem = db_list->headerItem();
        ___qtreewidgetitem->setText(0, QCoreApplication::translate("MainWindow", "\347\233\256\345\275\225", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableWidget_2->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("MainWindow", "id", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidget_2->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("MainWindow", "name", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tableWidget_2->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("MainWindow", "score", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = tableWidget_2->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("MainWindow", "rank", nullptr));

        const bool __sortingEnabled = tableWidget_2->isSortingEnabled();
        tableWidget_2->setSortingEnabled(false);
        QTableWidgetItem *___qtablewidgetitem4 = tableWidget_2->item(0, 0);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("MainWindow", "1", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = tableWidget_2->item(0, 1);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("MainWindow", "qwe", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = tableWidget_2->item(0, 2);
        ___qtablewidgetitem6->setText(QCoreApplication::translate("MainWindow", "12", nullptr));
        QTableWidgetItem *___qtablewidgetitem7 = tableWidget_2->item(0, 3);
        ___qtablewidgetitem7->setText(QCoreApplication::translate("MainWindow", "23", nullptr));
        QTableWidgetItem *___qtablewidgetitem8 = tableWidget_2->item(1, 0);
        ___qtablewidgetitem8->setText(QCoreApplication::translate("MainWindow", "2", nullptr));
        QTableWidgetItem *___qtablewidgetitem9 = tableWidget_2->item(2, 0);
        ___qtablewidgetitem9->setText(QCoreApplication::translate("MainWindow", "3", nullptr));
        QTableWidgetItem *___qtablewidgetitem10 = tableWidget_2->item(3, 0);
        ___qtablewidgetitem10->setText(QCoreApplication::translate("MainWindow", "4", nullptr));
        tableWidget_2->setSortingEnabled(__sortingEnabled);

        toolBar->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBar", nullptr));
        new_db->setTitle(QCoreApplication::translate("MainWindow", "\344\270\273\347\225\214\351\235\242", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
