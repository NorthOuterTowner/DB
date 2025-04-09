#include "loginwindow.h"
#include "./ui_loginwindow.h"
#include "usermanage.h"
#include "mainwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QSettings>

loginwindow::loginwindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::loginwindow)
    , user(new std::pair<std::string, std::string>("root", "root")) // 初始化成员
{
    ui->setupUi(this);
    //load username and password
    QSettings settings("MyDBMS", "LoginSettings");
    QString savedUsername = settings.value("username", "").toString();
    QString savedPassword = settings.value("password", "").toString();
    bool remember = settings.value("remember", false).toBool();

    // 设置初始界面
    if(remember && !savedPassword.isEmpty()){
        ui->user_line->setText(savedUsername);
        ui->code_line->setText(savedPassword);
        ui->rememberCheckBox->setChecked(true);
    }else{
        ui->user_line->setText(QString::fromStdString(user->first));
        ui->code_line->setText(QString::fromStdString(user->second));
    }
    
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

    connect(ui->rememberCheckBox,&QCheckBox::stateChanged,this,[=](int state){
        QSettings settings("MyDBMS", "LoginSettings");
        settings.setValue("remember",state == Qt::Checked);
    });
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
    user->first = username.toStdString();
    user->second = username.toStdString();
    if(username.isEmpty() || password.isEmpty()) {
        return;
    }

    if(UserManage::findUser(username.toStdString(), password.toStdString())) {
        QSettings settings("MyDBMS", "LoginSettings");
        bool remember = settings.value("remember", false).toBool();
        
        if (remember) {
            settings.setValue("username", username);
            settings.setValue("password", password);
        } else {
            settings.remove("username");
            settings.remove("password");
        }
        
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


void loginwindow::on_button_register_2_clicked()
{

}

