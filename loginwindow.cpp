#include "loginwindow.h"
#include "./ui_loginwindow.h"
#include "mainwindow.h"

loginwindow::loginwindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::loginwindow)
{
    ui->setupUi(this);
}

loginwindow::~loginwindow()
{
    delete ui;
}

void loginwindow::on_button_signin_clicked()
{
    MainWindow* w=new MainWindow;
    w->show();
    this->hide();
}

