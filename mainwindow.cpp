#include "mainwindow.h"
#include "highlighttextedit.h"
#include "./ui_mainwindow.h"
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <iostream>
#include "tablemanage.h"
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

    QIcon alter_icon("://res/image/alter_table.png");

    QIcon deltb_icon("://res/image/drop_table.png");

    QIcon newsql_icon("://res/image/new_SQL.png");

    QIcon alreadysql_icon("://res/image/already_SQL.png");

    QIcon start_icon("://res/image/yunx.png");

    QIcon save_icon("://res/image/save.png");

    QIcon clear_icon("://res/image/clear.png");

    //QIcon db_icon("://res/image/db.png");
    // 添加修改表的按钮和信号槽连接

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
    connect(new_table, &QAction::triggered, this, &MainWindow::onNewTableTriggered);

    QAction * alter = new QAction("修改表");
    alter->setIcon(alter_icon);
    connect(alter, &QAction::triggered, this, &MainWindow::onAlterTableTriggered);

    QAction * del_tb = new QAction("删除表");
    del_tb->setIcon(deltb_icon);
    connect(del_tb, &QAction::triggered, this, &MainWindow::onDropTableTriggered);

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
    ui->toolBar->addAction(alter);
    ui->toolBar->addAction(del_tb);
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

void MainWindow::onNewTableTriggered()
{
    bool ok1, ok2;
    QString dbName = QInputDialog::getText(this, tr("选择数据库"),
                                           tr("请输入数据库名称："),
                                           QLineEdit::Normal, "", &ok1);
    if (!ok1 || dbName.isEmpty()) {
        return;
    }

    QString tableName = QInputDialog::getText(this, tr("新建表"),
                                              tr("请输入表名称："),
                                              QLineEdit::Normal, "", &ok2);
    if (!ok2 || tableName.isEmpty()) {
        return;
    }

    QString sql = QString("CREATE TABLE %1;").arg(tableName);
    // 调试语句
    std::cout << "Creating new table with SQL: " << sql.toStdString() << std::endl;
    // 调用 Lexer 的处理方法，传递构造好的 SQL 语句
    std::string databaseName = dbName.toStdString();
    lexer.setCurrentDatabase(databaseName);
    lexer.handleRawSQL(sql);
}

void MainWindow::onAlterTableTriggered()
{
    bool ok1, ok2, ok3;
    QString dbName = QInputDialog::getText(this, tr("选择数据库"),
                                           tr("请输入数据库名称："),
                                           QLineEdit::Normal, "", &ok1);
    if (!ok1 || dbName.isEmpty()) {
        return;
    }

    QString tableName = QInputDialog::getText(this, tr("选择表"),
                                              tr("请输入表名称："),
                                              QLineEdit::Normal, "", &ok2);
    if (!ok2 || tableName.isEmpty()) {
        return;
    }

    QString operation = QInputDialog::getText(this, tr("选择操作"),
                                              tr("请输入操作类型 (ADD/MODIFY/DROP)："),
                                              QLineEdit::Normal, "", &ok3);
    if (!ok3 || operation.isEmpty()) {
        return;
    }

    std::vector<std::string> fieldNames;
    std::vector<int> fieldOrders;
    std::vector<std::string> fieldTypes;
    std::vector<int> fieldTypeParams;
    std::vector<std::string> constraints;

    if (operation == "ADD") {
        int numFields;
        bool ok4;
        numFields = QInputDialog::getInt(this, tr("添加字段数量"),
                                         tr("请输入要添加的字段数量："),
                                         0, 0, 100, 1, &ok4);
        if (!ok4 || numFields <= 0) {
            return;
        }

        for (int i = 0; i < numFields; ++i) {
            bool ok;
            QString fieldName = QInputDialog::getText(this, tr("添加字段"),
                                                      tr("请输入字段名："),
                                                      QLineEdit::Normal, "", &ok);
            if (!ok || fieldName.isEmpty()) {
                QMessageBox::warning(this, tr("输入错误"), tr("字段名不能为空，请重新输入！"));
                --i; // 让用户重新输入该字段名
                continue;
            }
            fieldNames.push_back(fieldName.toStdString());

            int fieldOrder = QInputDialog::getInt(this, tr("添加字段"),
                                                  tr("请输入字段顺序："),
                                                  0, 0, 1000, 1, &ok);
            if (!ok) {
                continue;
            }
            fieldOrders.push_back(fieldOrder);

            QString fieldType = QInputDialog::getText(this, tr("添加字段"),
                                                      tr("请输入字段类型："),
                                                      QLineEdit::Normal, "", &ok);
            if (!ok || fieldType.isEmpty()) {
                continue;
            }
            fieldTypes.push_back(fieldType.toStdString());

            int fieldTypeParam = QInputDialog::getInt(this, tr("添加字段"),
                                                      tr("请输入字段类型参数："),
                                                      0, 0, 1000, 1, &ok);
            if (!ok) {
                continue;
            }
            fieldTypeParams.push_back(fieldTypeParam);

            QString constraint = QInputDialog::getText(this, tr("添加字段"),
                                                       tr("请输入字段约束："),
                                                       QLineEdit::Normal, "", &ok);
            if (!ok) {
                constraint = "";
            }
            constraints.push_back(constraint.toStdString());
        }
    } else if (operation == "DROP") {
        int numFields;
        bool ok4;
        numFields = QInputDialog::getInt(this, tr("删除字段数量"),
                                         tr("请输入要删除的字段数量："),
                                         0, 0, 100, 1, &ok4);
        if (!ok4 || numFields <= 0) {
            return;
        }

        for (int i = 0; i < numFields; ++i) {
            bool ok;
            QString fieldName = QInputDialog::getText(this, tr("删除字段"),
                                                      tr("请输入要删除的字段名："),
                                                      QLineEdit::Normal, "", &ok);
            if (!ok || fieldName.isEmpty()) {
                QMessageBox::warning(this, tr("输入错误"), tr("字段名不能为空，请重新输入！"));
                --i; // 让用户重新输入该字段名
                continue;
            }
            fieldNames.push_back(fieldName.toStdString());
            // 对于删除操作，字段顺序、类型、参数和约束可以忽略
            fieldOrders.push_back(0);
            fieldTypes.push_back("");
            fieldTypeParams.push_back(0);
            constraints.push_back("");
        }

        // 提示用户确认删除操作
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("确认删除"), tr("你确定要删除这些字段吗？"),
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        }

        // 调试输出，查看 fieldNames 内容
        std::cout << "Fields to drop: ";
        for (const auto& field : fieldNames) {
            std::cout << field << " ";
        }
        std::cout << std::endl;
    }

    // 构造 SQL 语句
    QString sql;
    if (operation == "ADD") {
        sql = QString("ALTER TABLE %1 %2 ").arg(tableName).arg(operation);
        for (size_t i = 0; i < fieldNames.size(); ++i) {
            if (i > 0) {
                sql += ", ";
            }
            sql += QString::fromStdString(fieldNames[i]) + " " + QString::fromStdString(fieldTypes[i]);
            if (fieldTypeParams[i] > 0) {
                sql += QString("(%1)").arg(fieldTypeParams[i]);
            }
            if (!constraints[i].empty()) {
                sql += " " + QString::fromStdString(constraints[i]);
            }
        }
        sql += ";";
    } else if (operation == "DROP") {
        sql = QString("ALTER TABLE %1 %2 ").arg(tableName).arg(operation);
        for (size_t i = 0; i < fieldNames.size(); ++i) {
            if (i > 0) {
                sql += ", ";
            }
            sql += QString::fromStdString(fieldNames[i]);
        }
        sql += ";";
    } else {
        sql = QString("ALTER TABLE %1 %2;").arg(tableName).arg(operation);
    }

    // 调试语句
    std::cout << "Altering table with SQL: " << sql.toStdString() << std::endl;
    std::string databaseName = dbName.toStdString();
    lexer.setCurrentDatabase(databaseName);
    lexer.handleRawSQL(sql);
}

void MainWindow::onDropTableTriggered() {
    bool ok1, ok2;
    QString dbName = QInputDialog::getText(this, tr("选择数据库"),
                                           tr("请输入数据库名称："),
                                           QLineEdit::Normal, "", &ok1);
    if (!ok1 || dbName.isEmpty()) {
        return;
    }

    QString tableName = QInputDialog::getText(this, tr("选择表"),
                                              tr("请输入表名称："),
                                              QLineEdit::Normal, "", &ok2);
    if (!ok2 || tableName.isEmpty()) {
        return;
    }

    QString sql = QString("DROP TABLE %1;").arg(tableName);
    // 调试语句
    std::cout << "Dropping table with SQL: " << sql.toStdString() << std::endl;
    std::string databaseName = dbName.toStdString();
    lexer.setCurrentDatabase(databaseName);
    lexer.handleRawSQL(sql);
}
