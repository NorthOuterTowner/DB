#ifndef HIGHSETTINGS_H
#define HIGHSETTINGS_H

#include <QDialog>

namespace Ui {
class HighSettings;
}

class HighSettings : public QDialog
{
    Q_OBJECT

public:
    explicit HighSettings(QWidget *parent = nullptr);
    ~HighSettings();
    std::string getIP();
    std::string getPort();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::HighSettings *ui;
};

#endif // HIGHSETTINGS_H
