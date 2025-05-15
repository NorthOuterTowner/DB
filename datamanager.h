#ifndef DATAMANAGER_H
#define DATAMANAGER_H
#include <string>
#include <vector>
#include <map>
#include <variant>
#include<tablemanage.h>
#include <fieldmanage.h>
#include "affair.h"
#include <dbManager.h>

struct Node;//前向声明


class datamanager
{
public:
    datamanager(dbManager* dbMgr);
    ~datamanager();  // 确保在这里声明析构函数

    void backupDataFile(const std::string& dbName, const std::string& tableName);
    std::string getCurrentDatabase() const;//使其他模块访问当前数据库

    std::vector<std::vector<std::string>> readTableData(const std::string& dbName, const std::string& tableName);
    std::vector<std::string> getTableColumns(const std::string& dbName, const std::string& tableName);

    //std::string buildFilePath(const std::string& tableName);
    std::string buildFilePath(const std::string& dbName,const std::string& tableName);

    //void backupDataFile(const std::string& dbName, const std::string& tableName);

    bool insertData(const std::string& dbName,const std::string& tableName,const std::vector<std::string>& values);
    bool deleteData(const std::string& dbName,const std::string& tableName,const std::string& primaryKeyValue ) ;

    std::vector<std::vector<std::string>>selectData(const std::string& dbName,const std::string& tableName,

        const std::vector<std::string>& columnsToSelect, // 要选择的列名列表 (空表示选择所有列)
        const std::shared_ptr<Node>& whereTree,  // WHERE 子句的 AST 根节点 (由调用方解析并传入，可能为 nullptr)
        const std::vector<std::string>& aggregateFunctions, // 聚合函数列表
        const std::vector<std::string>& groupByColumns // GROUP BY 列名列表
                                                     );

    std::vector<std::string> calculateGlobalAggregates(
        const std::string& tableName,
        const std::vector<std::string>& aggregateFunctions, const std::shared_ptr<Node>& whereTree
        ) ;

    std::vector<std::vector<std::string>>selecData(const std::string& dbName,const std::string& tableName,
                                                     const std::vector<std::string>& columnsToSelect, // 要选择的列名列表 (空表示选择所有列)
                                                     const std::shared_ptr<Node>& whereTree,  // WHERE 子句的 AST 根节点 (由调用方解析并传入，可能为 nullptr)
                                                     const std::vector<std::string>& aggregateFunctions, // 聚合函数列表
                                                     const std::vector<std::string>& groupByColumns // GROUP BY 列名列表


                                                     );
    bool updateData( const std::string& dbName,
                    const std::string& tableName,
                    const std::map<std::string, std::string>& setMap,
                    const std::string& primaryKeyName,
                    const std::string& primaryKeyValue);

    /*bool updateData(
        const std::string& dbName,
        const std::string& tableName,
        const std::string& setClause, // 格式为 "col1=val1,col2=val2,..."
        const std::shared_ptr<Node>& whereTree // WHERE 子句的 AST 根节点 (可能为 nullptr)
        ) ;*/


    bool rollbackTransaction();

    bool commitTransaction();

    std::string getCurrentDateTime();

    bool backupDatabase(const std::string& dbName);;

    bool restoreDatabase(const std::string& dbName, const std::string& backupPath);

    //获取表信息
    tableManage::TableInfo getTableInfo(const std::string& dbName, const std::string& tableName);



    //类型转换
    bool tryStringtoInt(const std::string& s,int& out);
    bool tryStringtoDouble(const std::string& s,double& out);
    bool tryStringtoBool(const std::string& s,bool& out);

    // 验证插入数据的字段数量和类型
    bool validateInsertData(const tableManage::TableInfo& tableInfo,const std::vector<fieldManage::FieldInfo>& columnsInfo, const std::vector<std::string>& values);

    void updateTableRecordCount(const std::string& dbName, const std::string& tableName, int count);
    void updateTableLastModifiedDate(const std::string& dbName, const std::string& tableName);
    std::vector<std::string>splitString(const std::string& s, char delimiter) ;
    int findColumnIndex(const std::vector<fieldManage::FieldInfo>& columnsInfo, const std::string& columnName) ;

    bool convertValueForComparison(
        const std::string& valueStr,         // 要转换的字符串值
        const std::string& columnType,       // 列的数据类型
        int& intVal, double& doubleVal, bool& boolVal, std::string& stringVal // 输出参数，存储转换结果
        );

    bool evaluateWhereClauseTree(
        const std::shared_ptr<Node>& whereTree,                // WHERE 子句的 AST 根节点 (可能为 nullptr)
        const std::vector<std::string>& rowValues,           // 当前行的数据值 (字符串向量)
        const std::vector<fieldManage::FieldInfo>& columnsInfo // 表的所有列信息
        ) ;

    std::map<int, std::string> parseSetClause(
        const std::string& setClause,
        const std::vector<fieldManage::FieldInfo>& columnsInfo
        ) ;

    std::string joinRowValues(const std::vector<std::string>& rowValues) ;



private:
    tableManage tableMgr;
    fieldManage fieldMgr;
    Affair affair; // 声明 Affair 类型的成员变量
    dbManager* dbMgr;



};


#endif // DATAMANAGER_H
