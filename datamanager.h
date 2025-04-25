#ifndef DATAMANAGER_H
#define DATAMANAGER_H
#include <string>
#include <vector>
#include <map>
#include <variant>
#include<tablemanage.h>
#include <fieldmanage.h>



class datamanager
{
public:
    datamanager();

    bool insertData(const std::string& dbName,const std::string& tableName,const std::vector<std::string>& values);
    //获取表信息
    tableManage::TableInfo getTableInfo(const std::string& dbName, const std::string& tableName);


    bool tryStringtoInt(const std::string& s,int& out);
    bool tryStringtoDouble(const std::string& s,double& out);
    bool tryStringtoBool(const std::string& s,bool& out);
    // 验证插入数据的字段数量和类型

    bool validateInsertData(const tableManage::TableInfo& tableInfo,const std::vector<fieldManage::FieldInfo>& columnsInfo, const std::vector<std::string>& values);



    void updateTableRecordCount(const std::string& dbName, const std::string& tableName, int count);
    void updateTableLastModifiedDate(const std::string& dbName, const std::string& tableName);


private:
    tableManage tableMgr;
    fieldManage fieldMgr;



};


#endif // DATAMANAGER_H
