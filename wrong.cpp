#include "wrong.h"
#include "ui_wrong.h"

Wrong::Wrong(std::string info,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Wrong)
{
    ui->setupUi(this);
    ui->info->setText(QString::fromStdString(info));
    QPixmap* png = new QPixmap(":/res/icons/fatal.png");
    ui->png->setPixmap(*png);
}

Wrong::~Wrong()
{
    delete ui;
}
