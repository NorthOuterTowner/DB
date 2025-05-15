#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "lexer.h"
#include <QTableWidget>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNewDatabaseTriggered(); // 槽，用于处理按钮点击事件,完成新的数据库的建立
    void deleteDatabaseTriggered();
    void startTriggered();
    void onNewTableTriggered();
    void onAlterTableTriggered();
    void onDropTableTriggered();
    void onTableItemClicked(QTreeWidgetItem *item, int column);
    void onTableDefinitionChanged(const QString& tableName);
    void onTableHeaderClicked(int column);
    // MainWindow.h
private slots:
    void onSelectResultReceived(const std::vector<std::vector<std::string>>& rows, const QString& tableName);
    //void onOperationResultReceived(const QString& type, const QString& tableName, int count);
    // INSERT 结果处理
    void onInsertResultReceived(const QString& tableName, int rowsAffected);

    // UPDATE 结果处理
    void onUpdateResultReceived(const QString& tableName, int rowsAffected);

    // DELETE 结果处理
    void onDeleteResultReceived(const QString& tableName, int rowsAffected);

private:
    Ui::MainWindow *ui;
    Lexer lexer; // 数据库解析器实例
    QStringList readHeaderFromFile(const QString& filePath);
    void displaySelectResult(const std::vector<std::vector<std::string>>& rows,const QString& tableName );

//private:
 //   Ui::MainWindow *ui;
  //  Lexer lexer; // 数据库解析器实例
    dbManager* dbMgr;
    datamanager* dataMgr;//数据管理
};
#endif // MAINWINDOW_H
