#ifndef WRONG_H
#define WRONG_H

#include <QDialog>
#include <string>
#include <memory>

namespace Ui {
class Wrong;
}

class Wrong : public QDialog
{
    Q_OBJECT

public:
    // 删除拷贝构造函数和赋值运算符
    Wrong(const Wrong&) = delete;
    Wrong& operator=(const Wrong&) = delete;

    // 获取单例实例
    static Wrong* getInstance(const std::string& info = "");

    // 设置错误信息
    void setErrorInfo(const std::string& info);

    ~Wrong();

private:
    Ui::Wrong *ui;

    // 私有构造函数
    explicit Wrong(QWidget *parent = nullptr);

    // 静态单例指针
    static std::unique_ptr<Wrong> instance;
};

#endif // WRONG_H
