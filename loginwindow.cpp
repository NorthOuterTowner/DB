#include "loginwindow.h"
#include "./ui_loginwindow.h"
#include "usermanage.h"
#include "mainwindow.h"
#include <QMessageBox>
#include <QDebug>

loginwindow::loginwindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::loginwindow)
    , user(new std::pair<std::string, std::string>("root", "root")) // 初始化成员
{
    ui->setupUi(this);

    // 设置初始界面
    ui->user_line->setText(QString::fromStdString(user->first));
    ui->code_line->setText(QString::fromStdString(user->second));
    ui->code_line->setEchoMode(QLineEdit::Password); // 密码输入模式
    //ui->code_line->clear(); // 清空初始密码显示

    // 加载用户图标
    QPixmap userPix("://res/image/user.png");
    if(userPix.isNull()) {
        qDebug() << "用户图标加载失败";
    } else {
        ui->user_photo->setPixmap(userPix.scaled(
            ui->user_photo->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            ));
    }

    // 加载密码图标
    QPixmap codePix("://res/image/code.png");
    if(codePix.isNull()) {
        qDebug() << "密码图标加载失败";
    } else {
        ui->code_photo->setPixmap(codePix.scaled(
            ui->code_photo->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            ));
    }
}

loginwindow::~loginwindow()
{
    delete user;
    delete ui;
}

void loginwindow::on_button_signin_clicked()
{
    QString username = ui->user_line->text();
    QString password = ui->code_line->text();

    if(username.isEmpty() || password.isEmpty()) {
        return;
    }

    if(UserManage::findUser(username.toStdString(), password.toStdString())) {
        MainWindow *mainWin = new MainWindow();
        mainWin->show();
        this->close();
    }
}

void loginwindow::on_code_line_textChanged(const QString &arg1)
{
    user->second = arg1.toStdString();
    ui->code_line->setEchoMode(QLineEdit::Password);
}

void loginwindow::on_button_register_clicked()
{
    UserManage::createUser(user->first,user->second);
}


void loginwindow::on_user_line_textChanged(const QString &arg1)
{
    user->first = ui->user_line->text().toStdString();
}


void loginwindow::on_button_exit_clicked()
{
    QApplication::exit(0);
}

