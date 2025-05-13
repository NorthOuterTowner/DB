#include "session.h"

// 初始化静态成员变量
std::string Session::currentUserId;

// 设置当前用户 ID
void Session::setCurrentUserId(const std::string& userId) {
    currentUserId = userId;
}

// 获取当前用户 ID
std::string Session::getCurrentUserId() {
    return currentUserId;
}
