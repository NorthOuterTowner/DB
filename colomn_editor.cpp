#include "columneditor.h"

ColumnEditor::ColumnEditor(const QString &originalName, QWidget *parent) : QDialog(parent)
{
    setWindowTitle("编辑列名");
    setMinimumWidth(300);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *nameLayout = new QHBoxLayout;
    nameLayout->addWidget(new QLabel("列名:", this));
    nameEdit = new QLineEdit(originalName, this);
    nameLayout->addWidget(nameEdit);
    mainLayout->addLayout(nameLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *saveButton = new QPushButton("保存", this);
    QPushButton *cancelButton = new QPushButton("取消", this);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(saveButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

QString ColumnEditor::getNewName() const
{
    return nameEdit->text();
}

ColumnHeaderEditor::ColumnHeaderEditor(QTableWidget *tableWidget, QObject *parent)
    : QObject(parent), tableWidget(tableWidget)
{
    connect(tableWidget->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &ColumnHeaderEditor::onHeaderClicked);
}

void ColumnHeaderEditor::onHeaderClicked(int logicalIndex)
{
    QString currentName = tableWidget->horizontalHeaderItem(logicalIndex)->text();
    ColumnEditor dialog(currentName, tableWidget);

    if (dialog.exec() == QDialog::Accepted) {
        QString newName = dialog.getNewName();
        tableWidget->horizontalHeaderItem(logicalIndex)->setText(newName);
    }
}
