#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QInputDialog>
#include <QLineEdit>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , lexer(this) // 传递 this 指针给 Lexer 构造函数
{
    ui->setupUi(this);


    // 调用 lexer 的 setTreeWidget 方法设置 QTreeWidget 指针
    lexer.setTreeWidget(ui->db_list);
    lexer.reloadDbManagerDatabases(); // 调用新增的公共方法重新加载数据库信息
    //lexer.setTextEdit(ui->textEdit); // 新增设置 QTextEdit 指针用于SHOW DATABASES


    QIcon newdb_icon("://res/image/new_db.png");

    QIcon deldb_icon("://res/image/del_db.png");

    QIcon newtable_icon("://res/image/new_table.png");

    QIcon newsql_icon("://res/image/new_SQL.png");

    QIcon alreadysql_icon("://res/image/already_SQL.png");

    QIcon start_icon("://res/image/yunx.png");

    QIcon save_icon("://res/image/save.png");

    QIcon clear_icon("://res/image/clear.png");

    //QIcon db_icon("://res/image/db.png");


    //创建快捷项等同于菜单项
    QAction * new_db = new QAction("新建数据库");
    new_db->setIcon(newdb_icon);
    connect(new_db, &QAction::triggered, this, &MainWindow::onNewDatabaseTriggered);

    QAction * del_db = new QAction("删除数据库");
    del_db->setIcon(deldb_icon);

    // 连接按钮的 triggered 信号到自定义槽
    connect(del_db, &QAction::triggered, this, &MainWindow::deleteDatabaseTriggered);

    QAction * new_table = new QAction("新建表");
    new_table->setIcon(newtable_icon);

    QAction * new_SQL = new QAction("新建SQL文件");
    new_SQL->setIcon(newsql_icon);

    QAction * already_SQL = new QAction("打开已有的SQL文件");
    already_SQL->setIcon(alreadysql_icon);

    QAction * start = new QAction("运行SQL命令");
    start->setIcon(start_icon);
    connect(start, &QAction::triggered, this, &MainWindow::startTriggered);

    QAction * save = new QAction("保存至本地");
    save->setIcon(save_icon);

    QAction * clear = new QAction("清除SQL命令");
    clear->setIcon(clear_icon);

    //添加到工具栏中
    ui->toolBar->addAction(new_db);
    ui->toolBar->addAction(del_db);
    ui->toolBar->addAction(new_table);
    ui->toolBar->addAction(new_SQL);
    ui->toolBar->addAction(already_SQL);
    ui->toolBar->addAction(start);
    ui->toolBar->addAction(save);
    ui->toolBar->addAction(clear);

    //实现工具栏中的按钮的对应功能


    ui->frame->setStyleSheet("QFrame {"
                             "border: 2px solid black; "       // 设置边框颜色和宽度
                             "background-color: white; "   // 设置背景颜色
                             "border-radius: 10px; "           // 设置边框圆角
                             "}");

    ui->tableWidget_2->verticalHeader()->setVisible(false);
    connect(ui->exit_2,&QAction::triggered,this,[this](){
        QApplication::exit(0);
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onNewDatabaseTriggered()
{
    bool ok; // 声明一个布尔型变量用于跟踪用户是否确认输入

    // 弹出对话框，允许用户输入数据库名称
    QString dbName = QInputDialog::getText(this, tr("新建数据库"),
                                           tr("请输入数据库名称："),
                                           QLineEdit::Normal, "", &ok);
    // 检查用户是否点击了确定，并且输入不为空

    if (ok &&!dbName.isEmpty()) {
        // 构造完整的创建数据库SQL语句
        QString sql = QString("CREATE DATABASE %1;").arg(dbName);
        // 调试语句
        std::cout << "Creating new database with SQL: " << sql.toStdString() << std::endl;
        // 调用 Lexer 的处理方法，传递构造好的 SQL 语句
        lexer.handleRawSQL(sql);
    }


}

void MainWindow::deleteDatabaseTriggered()
{
    bool ok; // 声明一个布尔型变量用于跟踪用户是否确认输入

    // 弹出对话框，允许用户输入数据库名称
    QString dbName = QInputDialog::getText(this, tr("删除数据库"),
                                           tr("请输入数据库名称："),
                                           QLineEdit::Normal, "", &ok);
    // 检查用户是否点击了确定，并且输入不为空

    if (ok &&!dbName.isEmpty()) {
        // 构造完整的删除数据库SQL语句
        QString sql = QString("DROP DATABASE %1;").arg(dbName);
        // 调试语句
        std::cout << "Creating new database with SQL: " << sql.toStdString() << std::endl;
        // 调用 Lexer 的处理方法，传递构造好的 SQL 语句
        lexer.handleRawSQL(sql);
    }
}

void MainWindow::startTriggered()
{
    //从文本框读取用户输入的sql语句
    QString sql = ui->textEdit->toPlainText();

    // 确定用户输入不为空
    if (!sql.isEmpty()) {
        //调试语句
        std::cout << "SQL: " << sql.toStdString() << std::endl; // 输出 SQL 语句
        // 调用 Lexer 的处理方法，传递用户输入的 SQL 语句
        lexer.handleRawSQL(sql);

        // 清空 QTextEdit 中的内容
        ui->textEdit->clear();
    }
}
