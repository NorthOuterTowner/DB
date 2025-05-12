#include "logger.h"
#include <iostream>

Logger::Logger(const std::string& logFilePath) {
    logFile.open(logFilePath, std::ios::app); // 以追加模式打开文件
    if (!logFile.is_open()) {
        std::cerr << "Error opening log file: " << logFilePath << std::endl;
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close(); // 在析构时关闭文件流
    }
}

std::string Logger::getCurrentTimestamp() {
    std::time_t now = std::time(nullptr); // 获取当前时间
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now)); // 格式化时间戳
    return std::string(buffer);
}

void Logger::log(const std::string& userId, const std::string& actionType, const std::string& objectType, const std::string& detail) {
    if (logFile.is_open()) {
        logFile << getCurrentTimestamp() << ", " << userId << ", " << actionType << ", " << objectType << ", " << detail << std::endl;
    } else {
        std::cerr << "Log file is not open." << std::endl;
    }
}

// 新增的登录日志记录函数
void Logger::logLogin(const std::string& userId, const std::string& ipAddress) {
    log(userId, "Login", "User", "IP: " + ipAddress);
}

// 新增的登出日志记录函数
void Logger::logLogout(const std::string& userId, const std::string& ipAddress) {
    log(userId, "Logout", "User", "IP: " + ipAddress);
}
