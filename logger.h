#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <ctime>

class Logger {
public:
    Logger(const std::string& logFilePath); // 构造函数
    ~Logger(); // 析构函数
    void log(const std::string& userId, const std::string& actionType, const std::string& objectType, const std::string& detail); // 用于数据库、表相关的日志记录
    void logLogin(const std::string& userId, const std::string& ipAddress); //用于用户登录的日志记录
    void logLogout(const std::string& userId, const std::string& ipAddress); //用于用户退出的日志记录

private:
    std::ofstream logFile; // 用于写入日志的文件流
    std::string getCurrentTimestamp(); // 获取当前时间戳
};

#endif // LOGGER_H
