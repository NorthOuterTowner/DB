#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    QIcon newdb_icon("://res/image/new_db.png");

    QIcon newtable_icon("://res/image/new_table.png");

    QIcon newsql_icon("://res/image/new_SQL.png");

    QIcon alreadysql_icon("://res/image/already_SQL.png");

    QIcon start_icon("://res/image/yunx.png");

    QIcon save_icon("://res/image/save.png");

    QIcon clear_icon("://res/image/clear.png");


    //创建快捷项等同于菜单项
    QAction * new_db = new QAction("新建数据库");
    new_db->setIcon(newdb_icon);


    QAction * new_table = new QAction("新建表");
    new_table->setIcon(newtable_icon);

    QAction * new_SQL = new QAction("新建SQL文件");
    new_SQL->setIcon(newsql_icon);

    QAction * already_SQL = new QAction("打开已有的SQL文件");
    already_SQL->setIcon(alreadysql_icon);

    QAction * start = new QAction("运行SQL命令");
    start->setIcon(start_icon);

    QAction * save = new QAction("保存至本地");
    save->setIcon(save_icon);

    QAction * clear = new QAction("清除SQL命令");
    clear->setIcon(clear_icon);

    //添加到工具栏中
    ui->toolBar->addAction(new_db);
    ui->toolBar->addAction(new_table);
    ui->toolBar->addAction(new_SQL);
    ui->toolBar->addAction(already_SQL);
    ui->toolBar->addAction(start);
    ui->toolBar->addAction(save);
    ui->toolBar->addAction(clear);


    ui->frame->setStyleSheet("QFrame {"
                             "border: 2px solid black; "       // 设置边框颜色和宽度
                             "background-color: white; "   // 设置背景颜色
                             "border-radius: 10px; "           // 设置边框圆角
                             "}");

}

MainWindow::~MainWindow()
{
    delete ui;
}
