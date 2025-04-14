#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>

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


