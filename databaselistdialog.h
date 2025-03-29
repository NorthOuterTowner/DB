#ifndef DATABASELISTDIALOG_H
#define DATABASELISTDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QStringList>

class DatabaseListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DatabaseListDialog(const QStringList& databaseNames, QWidget *parent = nullptr);
    ~DatabaseListDialog();

private:
    QListWidget *listWidget;
};

#endif // DATABASELISTDIALOG_H
