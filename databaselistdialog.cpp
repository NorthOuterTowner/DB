#include "databaselistdialog.h"
#include <QVBoxLayout>

DatabaseListDialog::DatabaseListDialog(const QStringList& databaseNames, QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Database");
    listWidget = new QListWidget(this);
    for (const QString& name : databaseNames) {
        listWidget->addItem(name);
    }

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(listWidget);
    setLayout(layout);
}

DatabaseListDialog::~DatabaseListDialog()
{

}
