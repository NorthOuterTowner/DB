#ifndef FIELDINPUTDIALOG_H
#define FIELDINPUTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QPushButton>
#include <QTabWidget>
#include <optional>

struct FieldData {
    QString name;
    QString type;
    int param;
    QString constraint;
    QString defaultValue;
};

class FieldInputDialog : public QDialog {
    Q_OBJECT

public:
    explicit FieldInputDialog(QWidget *parent = nullptr);
    explicit FieldInputDialog(const QString& tableName, const QString& fieldName, QWidget *parent);

    QList<FieldData> getFields() const;
    QTabWidget* getTabWidget() const { return tabWidget; }
    void setName(const QString& name);
    bool loadFieldData(const QString& tableName, const QString& fieldName);
private:
    // 从表定义文件中获取字段信息
    std::optional<FieldData> getFieldDataFromDefinition(const QString& tableName, const QString& fieldName);

    // 设置字段信息到当前活动标签页
    void setFieldDataToCurrentTab(const FieldData& data);

private slots:
    void addFieldTab();
    void removeCurrentTab();
    void tabChanged(int index);
    void updateTabText(int index, const QString &fieldName);
    void updateAllTabTexts();

private:
    QVBoxLayout *mainLayout;
    QTabWidget *tabWidget;
    QPushButton *addButton;
    QPushButton *removeButton;
    QDialogButtonBox *buttonBox;
    QPushButton *deleteButton;

};

#endif // FIELDINPUTDIALOG_H
