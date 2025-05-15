#include "mainwindow.h"
#include "highlighttextedit.h"
#include "./ui_mainwindow.h"
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <iostream>
#include "tablemanage.h"
#include "FieldInputDialog.h"
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
    ui->toolBar->addAction(start);


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

    // 连接 QTreeWidget 的 itemClicked 信号到槽函数
    connect(ui->db_list, &QTreeWidget::itemClicked, this, &MainWindow::onTableItemClicked);
    connect(&lexer, &Lexer::tableDefinitionChanged, this, &MainWindow::onTableDefinitionChanged);
    connect(ui->tableWidget_2->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &MainWindow::onTableHeaderClicked);

    connect(&lexer, &Lexer::sendSelectResult, this, &MainWindow::displaySelectResult);

    dbMgr=new dbManager();
    dataMgr=new datamanager(dbMgr);

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

    QString operation = QInputDialog::getText(this, tr("选择操作"),
                                              tr("请输入操作类型 (ADD/MODIFY/DROP)："),
                                              QLineEdit::Normal, "", &ok2);
    if (!ok2 || operation.isEmpty()) {
        return;
    }

    // 打开字段编辑对话框
    FieldInputDialog dialog(this);
    dialog.setWindowTitle(tr("修改表结构 - %1").arg(operation));

    if (dialog.exec() == QDialog::Accepted) {
        QList<FieldData> fields = dialog.getFields();

        if (fields.isEmpty()) {
            QMessageBox::warning(this, tr("操作取消"), tr("未添加任何字段!"));
            return;
        }

        // 构造SQL语句
        QString sql;
        if (operation == "ADD") {
            sql = QString("ALTER TABLE %1 ADD ").arg(tableName);
            for (int i = 0; i < fields.size(); ++i) {
                if (i > 0) sql += ", ";
                const FieldData &field = fields[i];
                sql += field.name + " " + field.type;
                if (field.param > 0) {
                    sql += QString("(%1)").arg(field.param);
                }
                if (!field.constraint.isEmpty()) {
                    sql += " " + field.constraint;
                }
                if (!field.defaultValue.isEmpty()) {
                    if (field.type == "VARCHAR" || field.type == "TEXT" || field.type == "DATETIME") {
                        sql += " DEFAULT '" + field.defaultValue + "'";
                    } else {
                        sql += " DEFAULT " + field.defaultValue;
                    }
                }
            }
            sql += ";";
        } else if (operation == "MODIFY") {
            sql = QString("ALTER TABLE %1 MODIFY ").arg(tableName);
            for (int i = 0; i < fields.size(); ++i) {
                if (i > 0) sql += ", ";
                const FieldData &field = fields[i];
                sql += field.name + " " + field.type;
                if (field.param > 0) {
                    sql += QString("(%1)").arg(field.param);
                }
                if (!field.constraint.isEmpty()) {
                    sql += " " + field.constraint;
                }
                if (!field.defaultValue.isEmpty()) {
                    if (field.type == "VARCHAR" || field.type == "TEXT" || field.type == "DATETIME") {
                        sql += " DEFAULT '" + field.defaultValue + "'";
                    } else {
                        sql += " DEFAULT " + field.defaultValue;
                    }
                }
            }
            sql += ";";
        } else if (operation == "DROP") {
            sql = QString("ALTER TABLE %1 DROP COLUMN ").arg(tableName);
            for (int i = 0; i < fields.size(); ++i) {
                if (i > 0) sql += ", ";
                sql += fields[i].name;
            }
            sql += ";";
        }

        // 执行SQL
        std::cout << "Altering table with SQL: " << sql.toStdString() << std::endl;
        std::string databaseName = dbName.toStdString();
        lexer.setCurrentDatabase(databaseName);
        lexer.handleRawSQL(sql);
    }
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

void MainWindow::onTableItemClicked(QTreeWidgetItem *item, int column)
{
    // 清空tableWidget_2的所有内容
    ui->tableWidget_2->clear();
    ui->tableWidget_2->setRowCount(0);
    ui->tableWidget_2->setColumnCount(0);

    // 获取被选中的表名
    QString tableName = item->text(0);
    std::string tableNameStr = tableName.toStdString();

    // 假设表定义文件路径为 "../../res/表名.tdf.txt"
    std::string tableDefFilePath = "../../res/" + tableNameStr + ".tdf.txt";
    QFile tableDefFile(QString::fromStdString(tableDefFilePath));

    if (tableDefFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&tableDefFile);
        QString line;
        std::vector<std::string> fieldNames;

        // 读取表定义文件的每一行，提取字段名
        while ((line = in.readLine()) != "") {
            std::vector<std::string> parts = lexer.split(line.toStdString(), " ");
            if (parts.size() > 2) {
                fieldNames.push_back(parts[2]);
            }
        }
        tableDefFile.close();

        // 设置tableWidget_2的列数为字段名的数量
        ui->tableWidget_2->setColumnCount(static_cast<int>(fieldNames.size()));

        // 将字段名设置为tableWidget_2的列名
        for (size_t i = 0; i < fieldNames.size(); ++i) {
            QTableWidgetItem *headerItem = new QTableWidgetItem(QString::fromStdString(fieldNames[i]));
            ui->tableWidget_2->setHorizontalHeaderItem(static_cast<int>(i), headerItem);
        }

        // 读取表数据并填充到tableWidget_2中
        std::string tableDataFilePath = "../../res/" + tableNameStr + ".data.txt";
        QFile tableDataFile(QString::fromStdString(tableDataFilePath));

        if (tableDataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream dataIn(&tableDataFile);
            int row = 0;

            while (!dataIn.atEnd()) {
                QString line = dataIn.readLine();
                QStringList values = line.split(" ");

                // 确保数据行的列数与表头列数一致
                if (values.size() == fieldNames.size()) {
                    ui->tableWidget_2->insertRow(row);

                    for (int col = 0; col < values.size(); ++col) {
                        QTableWidgetItem *item = new QTableWidgetItem(values[col]);
                        ui->tableWidget_2->setItem(row, col, item);
                    }

                    row++;
                }
            }

            tableDataFile.close();
        }
    } else {
        std::cerr << "Failed to open table definition file: " << tableDefFilePath << std::endl;
        // 显示空表格
        ui->tableWidget_2->setColumnCount(0);
        ui->tableWidget_2->setRowCount(0);
    }
}

void MainWindow::onTableDefinitionChanged(const QString& tableName)
{
    // 创建一个虚拟的 QTreeWidgetItem 用于传递表名
    QTreeWidgetItem* dummyItem = new QTreeWidgetItem();
    dummyItem->setText(0, tableName);

    // 调用 onTableItemClicked 函数
    onTableItemClicked(dummyItem, 0);

    // 释放虚拟的 QTreeWidgetItem
    delete dummyItem;
}

void MainWindow::onTableHeaderClicked(int column)
{
    // 获取点击的列名
    QString fieldName = ui->tableWidget_2->horizontalHeaderItem(column)->text();

    // 获取当前选中的表名
    QTreeWidgetItem *currentItem = ui->db_list->currentItem();
    if (!currentItem) return;

    QString tableName = currentItem->text(0);

    // 以编辑模式打开对话框
    FieldInputDialog dialog(tableName, fieldName, this);

    if (dialog.exec() == QDialog::Accepted) {
        // 处理对话框的accepted信号，更新字段信息
        QList<FieldData> fields = dialog.getFields();
        if (!fields.isEmpty()) {
            FieldData field = fields.first();

            // 从当前项获取数据库名
            QTreeWidgetItem *parentItem = currentItem->parent();
            if (parentItem) {
                QString dbName = parentItem->text(0);

                // 调用fieldManage类来修改字段
                fieldManage fm;
                bool success = fm.modifyField(
                    dbName.toStdString(),
                    tableName.toStdString(),
                    field.name.toStdString(),
                    column, // 使用列索引作为字段顺序
                    field.type.toStdString(),
                    field.param,
                    field.constraint.toStdString()
                    );

                if (success) {
                    // 更新表格显示
                    onTableDefinitionChanged(tableName);
                    QMessageBox::information(this, tr("成功"), tr("字段修改成功"));
                } else {
                    QMessageBox::warning(this, tr("失败"), tr("字段修改失败"));
                }
            }
        }
    }
}

void MainWindow::displaySelectResult(const std::vector<std::vector<std::string>>& rows) {
    QTableWidget* table = ui->tableWidget_2;
    table->clear(); // 清空旧数据
    table->setRowCount(0);
    table->setColumnCount(0);

    if (rows.empty()) return; // 没有数据

    int columnCount = static_cast<int>(rows[0].size());
    table->setColumnCount(columnCount);

    for (int row = 0; row < rows.size(); ++row) {
        table->insertRow(row);
        for (int col = 0; col < columnCount; ++col) {
            QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(rows[row][col]));
            table->setItem(row, col, item);
        }
    }

    table->resizeColumnsToContents(); // 自动调整列宽
}

