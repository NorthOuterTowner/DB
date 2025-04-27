#ifndef DATAMANAGER_H
#define DATAMANAGER_H
#include <string>
#include <vector>
#include <map>
#include <variant>
#include<tablemanage.h>
#include <fieldmanage.h>

struct Node;//前向声明
 std::string buildFilePath(const std::string& dbName, const std::string& tableName) ;

class datamanager
{
public:
    datamanager();

    bool insertData(const std::string& dbName,const std::string& tableName,const std::vector<std::string>& values);
    bool deleteData(const std::string& dbName, const std::string& tableName, const std::vector<int>& rowIndicesToDelete);
    std::vector<std::vector<std::string>>selectData(
        const std::string& dbName,
        const std::string& tableName,
        const std::vector<std::string>& columnsToSelect, // 要选择的列名列表 (空表示选择所有列)
        const std::shared_ptr<Node>& whereTree  // WHERE 子句的 AST 根节点 (由调用方解析并传入，可能为 nullptr)
        );
    bool updateData(
        const std::string& dbName,
        const std::string& tableName,
        const std::string& setClause,    // SET 子句字符串 (例如 "column1=value1, column2=value2")
        const std::shared_ptr<Node>& whereTree // WHERE 子句的 AST 根节点 (由调用方解析并传入，可能为 nullptr)
        ) ;

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



};


#endif // DATAMANAGER_H
