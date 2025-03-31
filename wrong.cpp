#include "wrong.h"
#include "ui_wrong.h"
#include <QPixmap>

// 初始化静态成员
std::unique_ptr<Wrong> Wrong::instance = nullptr;

Wrong* Wrong::getInstance(const std::string& info)
{
    if (!instance) {
        instance.reset(new Wrong());
    }
    if (!info.empty()) {
        instance->setErrorInfo(info);
    }
    return instance.get();
}

Wrong::Wrong(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Wrong)
{
    ui->setupUi(this);

    // 设置默认图标
    QPixmap png(":/res/icons/fatal.png");
    if (!png.isNull()) {
        ui->png->setPixmap(png);
    }

    // 窗口属性设置
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setModal(true);
}

Wrong::~Wrong()
{
    delete ui;
}

void Wrong::setErrorInfo(const std::string& info)
{
    ui->info->setText(QString::fromStdString(info));
}
