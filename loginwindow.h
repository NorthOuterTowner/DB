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

private slots:
    void on_button_signin_clicked();
    void on_code_line_textChanged(const QString &arg1);

private:
    Ui::loginwindow *ui;
    std::pair<std::string,std::string>* user;
    std::string showCode;
};
#endif // LOGINWINDOW_H


