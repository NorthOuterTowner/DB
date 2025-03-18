#include "loginwindow.h"
#include "./ui_loginwindow.h"
#include "mainwindow.h"

loginwindow::loginwindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::loginwindow)
{
    ui->setupUi(this);

    QPixmap pixmap("://res/image/user.png");
    if (pixmap.isNull()) {
        qDebug() << "图片加载失败！";
    } else {
        pixmap = pixmap.scaled(ui->user_photo->geometry().size());
        ui->user_photo->setPixmap(pixmap);

    }

    QPixmap pixmap1("://res/image/code.png");
    if (pixmap1.isNull()) {
        qDebug() << "图片加载失败！";
    } else {
        pixmap1 = pixmap1.scaled(ui->user_photo->geometry().size());
        ui->code_photo->setPixmap(pixmap1);
    }



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

