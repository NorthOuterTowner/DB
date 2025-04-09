#include "highsettings.h"
#include "ui_highsettings.h"

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
    this->hide();
}
