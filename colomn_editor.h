#ifndef COLUMNEDITOR_H
#define COLUMNEDITOR_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>

class ColumnEditor : public QDialog
{
    Q_OBJECT
public:
    explicit ColumnEditor(const QString &originalName, QWidget *parent = nullptr);
    QString getNewName() const;

private:
    QLineEdit *nameEdit;
};

class ColumnHeaderEditor : public QObject
{
    Q_OBJECT
public:
    explicit ColumnHeaderEditor(QTableWidget *tableWidget, QObject *parent = nullptr);

private slots:
    void onHeaderClicked(int logicalIndex);

private:
    QTableWidget *tableWidget;
};

#endif // COLUMNEDITOR_H
