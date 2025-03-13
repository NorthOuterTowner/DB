#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "lexer.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Lexer *lexer = new Lexer();
    connect(ui->pushButton, &QPushButton::clicked, this, [=]() {
        QString rawSQL = ui->inputSQL->toPlainText();
        lexer->handleRawSQL(rawSQL);  // 调用 Lexer 的槽函数
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    std::string rawSQL=ui->inputSQL->toPlainText().toStdString();

}

