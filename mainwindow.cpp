#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    //添加图标
    QIcon yunx_icon("://res/image/yunx.png");
    ui->yunx->setIcon(yunx_icon);

    QIcon newdb_icon("://res/image/new_db.png");
    ui->newdb->setIcon(newdb_icon);

    QIcon newtable_icon("://res/image/new_table.png");
    ui->newtable->setIcon(newtable_icon);

    QIcon clear_icon("://res/image/clear.png");
    ui->clear->setIcon(clear_icon);

    QIcon exit_icon("://res/image/exit.png");
    ui->exit->setIcon(exit_icon);

    QIcon save_icon("://res/image/save.png");
    ui->save->setIcon(save_icon);



}

MainWindow::~MainWindow()
{
    delete ui;
}
