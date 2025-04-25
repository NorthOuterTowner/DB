#include "datamanager.h"
#include <fstream>
#include <iostream>
#include <sstream>


datamanager::datamanager() {}



bool datamanager::tryStringtoInt(const std::string& s,int& out){
    if(s.empty())return false;
    try{
        size_t pos=0;
        out=std::stoi(s,&pos);
        //检查是否整个字符串都被转换了 并且没有剩余非数字字符
        return pos==s.size();
    }catch (const std::invalid_argument& ia){
        //无效参数 不是一个有效的数字格式
        return false;
    }catch(const std::out_of_range& oor){
        //超出int范围
        return false;
    }
}

bool datamanager::tryStringtoDouble(const std::string& s,double& out){
    if(s.empty())return false;
    try{
        size_t pos=0;
        out = std::stod(s,&pos);
        //检查是否整个字符串都被替换了
        return pos==s.size();
    }catch(const std::invalid_argument& ia){
        return false;
    }catch (const std::out_of_range& oor){
        return false;
    }
}

bool datamanager::tryStringtoBool(const std::string& s,bool& out){
    //简化处理：接收 true、false、1、0
    std::string lower_s;
    lower_s.resize(s.size());
    std::transform(s.begin(),s.end(),s.begin(),::tolower);

    if(lower_s=="true"||lower_s =="1"){
        out=true;
        return true;
    }else if(lower_s=="false"||lower_s=="0"){
        out=false;
        return true;
    }
    return false;

}


bool datamanager::insertData(const std::string& dbName,const std::string& tableName,const std::vector<std::string>& values){
    // 获取表信息
    tableManage::TableInfo tableInfo = tableMgr.getTableInfo(dbName, tableName);
    if (tableInfo.table_name.empty()) {
        std::cerr << "Table " << tableName << " does not exist in database " << dbName << std::endl;
        return false;
    }

    //获取表的详细列信息
    std::vector<fieldManage::FieldInfo>columnsInfo=fieldMgr.getFieldsInfo(tableName);

    // 验证插入数据
    if (!validateInsertData(tableInfo,columnsInfo, values)) {
        std::cerr << "Invalid data for table " << tableName << std::endl;
        return false;
    }

    // 构建数据文件路径
    std::string dataFilePath = "../../res/" + tableName + ".data.txt";

    // 打开数据文件以追加模式写入
    std::ofstream dataFile(dataFilePath, std::ios::app);
    if (!dataFile.is_open()) {
        std::cerr << "Failed to open data file: " << dataFilePath << std::endl;
        return false;
    }

    // 写入数据
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            dataFile << ",";
        }
        dataFile << values[i];
    }
    dataFile << std::endl;

    // 关闭数据文件
    dataFile.close();

    // 更新表的记录数
    updateTableRecordCount(dbName, tableName, 1);

    // 更新表的最后修改时间
    updateTableLastModifiedDate(dbName, tableName);

    return true;
}


bool datamanager::validateInsertData(const tableManage::TableInfo& tableInfo,const std::vector<fieldManage::FieldInfo>& columnsInfo, const std::vector<std::string>& values){

    //检查值的数量是否与列数一致
    if(values.size()!=columnsInfo.size()!=columnsInfo.size()){
        std::cerr << "Validation Error: Number of values (" << values.size()
        << ") does not match the number of columns (" << columnsInfo.size()
        << ") in table '" << tableInfo.table_name << "'." << std::endl;
        return false;

    }
    // 逐个值进行检查（NOT NULL 和数据类型）
    for (size_t i = 0; i < values.size(); ++i) {
        const fieldManage::FieldInfo& column = columnsInfo[i];
        const std::string& value = values[i];

        // 检查 NOT NULL 约束
        bool isNotNull = (column.constraints.find("NOT NULL")!=std::string::npos);
        if (isNotNull && value.empty()) {
            std::cerr << "Validation Error: Column '" << column.fieldName
                      << "' (index " << i << ") cannot be NULL (empty string)." << std::endl;
            return false;
        }

        // 如果值不为空，则进行数据类型检查
        if (!value.empty()) {
            // 将列类型转换为大写以便比较
            std::string columnTypeUpper = column.fieldType;
            std::transform(columnTypeUpper.begin(), columnTypeUpper.end(), columnTypeUpper.begin(), ::toupper);

            if (columnTypeUpper == "INT") {
                int int_val;
                if (!tryStringtoInt(value, int_val)) {
                    std::cerr << "Validation Error: Value '" << value
                              << "' for column '" << column.fieldName << "' (index " << i
                              << ") is not a valid INT." << std::endl;
                    return false;
                }
            } else if (columnTypeUpper == "DOUBLE" || columnTypeUpper == "FLOAT") {
                double double_val;
                if (!tryStringtoDouble(value, double_val)) {
                    std::cerr << "Validation Error: Value '" << value
                              << "' for column '" << column.fieldName << "' (index " << i
                              << ") is not a valid DOUBLE/FLOAT." << std::endl;
                    return false;
                }
            } else if (columnTypeUpper == "BOOL" || columnTypeUpper == "BOOLEAN") {
                bool bool_val;
                if (!tryStringtoBool(value, bool_val)) {
                    std::cerr << "Validation Error: Value '" << value
                              << "' for column '" << column.fieldName << "' (index " << i
                              << ") is not a valid BOOL (expected 'true', 'false', '0', or '1')." << std::endl;
                    return false;
                }
            }
            // 对于 VARCHAR/TEXT 类型，如果值不为空且 NOT NULL 约束已通过，则认为有效
            // 如果需要限制 VARCHAR 长度，可以在这里添加检查
            else if (columnTypeUpper == "VARCHAR" || columnTypeUpper == "TEXT") {
                // 基本验证已通过（非空且不是 NOT NULL）
            }
            //添加更多数据类型的检查，例如 DATE, TIME 等...
            else {
                // 如果遇到未知的数据类型，可以视为错误或跳过验证（取决于你的需求）
                std::cerr << "Validation Warning: Unknown data type '" << column.fieldType
                          << "' for column '" << column.fieldName << "' (index " << i
                          << "). Skipping type validation for this column." << std::endl;
                // return false; // 如果未知类型视为错误
            }
        }
    }

    // 如果所有检查都通过，则数据有效
    return true;




}
