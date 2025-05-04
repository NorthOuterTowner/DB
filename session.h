//用于获取当前登录的用户信息便于日志记录
#ifndef SESSION_H
#define SESSION_H

#include <string>

class Session {
public:
    // 设置当前用户 ID
    static void setCurrentUserId(const std::string& userId);

    // 获取当前用户 ID
    static std::string getCurrentUserId();

private:
    static std::string currentUserId; // 存储当前用户 ID
};

#endif // SESSION_H
