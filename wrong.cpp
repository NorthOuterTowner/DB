#include "wrong.h"
#include "ui_wrong.h"

Wrong::Wrong(std::string info,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Wrong)
{
    ui->setupUi(this);
    ui->info->setText(QString::fromStdString(info));
}

Wrong::~Wrong()
{
    delete ui;
}
