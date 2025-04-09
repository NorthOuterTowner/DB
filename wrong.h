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
    Wrong(const Wrong&) = delete;
    Wrong& operator=(const Wrong&) = delete;
    static Wrong* getInstance(const std::string& info = "");
    void setErrorInfo(const std::string& info);
    ~Wrong();

private:
    Ui::Wrong *ui;
    explicit Wrong(QWidget *parent = nullptr);
    static std::unique_ptr<Wrong> instance;
};

#endif // WRONG_H
