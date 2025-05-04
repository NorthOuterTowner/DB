#include "loginwindow.h"
#include "loginwindow.h"
#include "logger.h"
#include "./ui_loginwindow.h"
#include "usermanage.h"
#include "mainwindow.h"
#include "session.h"
#include <QMessageBox>
#include <QDebug>
#include <QSettings>
#include "highsettings.h"
#include "server.h"
#include <iostream>

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

// 定义类的成员函数
QString loginwindow::getIPAddress() {
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // 获取第一个非本地地址的 IPv4 地址
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // 如果没有找到非本地地址的 IPv4 地址，则使用本地地址
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    return ipAddress;
}

void loginwindow::on_button_signin_clicked()
{
    QString username = ui->user_line->text();
    QString password = ui->code_line->text();
    user->first = username.toStdString();

    //这里好像错了，应该把password赋值
    //user->second = username.toStdString();
    user->second = password.toStdString();

    if(username.isEmpty() || password.isEmpty()) {
        return;
    }

    if(UserManage::findUser(username.toStdString(), password.toStdString())) {

        //添加此句用于数据库和表等操作时的日志记录
        Session::setCurrentUserId(username.toStdString()); // 这里设置当前用户 ID

        QSettings settings("MyDBMS", "LoginSettings");
        bool remember = settings.value("remember", false).toBool();

        if (remember) {
            settings.setValue("username", username);
            settings.setValue("password", password);
        } else {
            settings.remove("username");
            settings.remove("password");
        }

        // 记录登录日志
        Logger logger("../../res/system_logs.txt");
        QString ip = getIPAddress();
        logger.logLogin(username.toStdString(), ip.toStdString());

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
    std::string userId = Session::getCurrentUserId();
    if (!userId.empty()) {
        Logger logger("../../res/system_logs.txt");
        QString ip = getIPAddress();
        logger.logLogout(userId, ip.toStdString());
    }
    QApplication::exit(0);
}


void loginwindow::on_button_register_2_clicked()
{
    HighSettings *h = new HighSettings();
    h->show();
}
