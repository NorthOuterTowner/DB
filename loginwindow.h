#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QNetworkInterface> // 添加到这里，因为类中需要使用这个头文件中的类型

QT_BEGIN_NAMESPACE
namespace Ui {
class loginwindow;
}
QT_END_NAMESPACE

class loginwindow : public QMainWindow
{
    Q_OBJECT

public:
    loginwindow(QWidget *parent = nullptr);
    ~loginwindow();
    inline std::string getUserName(){
        return this->user->first;
    }
    QString getIPAddress(); // 声明类的成员函数

private slots:
    void on_button_signin_clicked();
    void on_code_line_textChanged(const QString &arg1);
    void on_button_register_clicked();
    void on_user_line_textChanged(const QString &arg1);
    void on_button_exit_clicked();
    void on_button_register_2_clicked();

private:
    Ui::loginwindow *ui;
    std::pair<std::string,std::string>* user;
    std::string showCode;
};

#endif // LOGINWINDOW_H
