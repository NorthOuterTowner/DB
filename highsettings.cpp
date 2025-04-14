#include "highsettings.h"
#include "ui_highsettings.h"
#include "server.h"
#include <iostream>

HighSettings::HighSettings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::HighSettings)
{
    ui->setupUi(this);
}

HighSettings::~HighSettings()
{
    delete ui;
}

std::string HighSettings::getIP(){
    return ui->ipedit->text().toStdString();
}

std::string HighSettings::getPort(){
    return ui->portedit->text().toStdString();
}

void HighSettings::on_buttonBox_accepted()
{
    std::string ip = this->getIP();
    int port = std::stoi(this->getPort());
    Server::getInstance()->setIP(ip);
    Server::getInstance()->setPort(port);
    this->hide();
}

void HighSettings::on_buttonBox_rejected()
{
    this->hide();
}

