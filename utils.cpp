#include "utils.h"
#include <vector>
#include <string>
#include <algorithm>
utils::utils() {}

std::vector<std::string> utils::split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0; // 子字符串的起始位置
    size_t end = str.find(delimiter); // 查找分隔符的位置

    // 循环查找分隔符并分割字符串
    while (end != std::string::npos) {
        std::string token = str.substr(start, end - start); // 提取子字符串
        if (!token.empty()) { // 忽略空字符串
            tokens.push_back(token);
        }
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }

    std::string token = str.substr(start);
    if (!token.empty()) {
        tokens.push_back(token);
    }

    return tokens;
}

std::string utils::strip(const std::string& str) {
    auto start = str.find_first_not_of(" \t\r\n");
    auto end = str.find_last_not_of(" \t\r\n");

    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}


std::string utils::toUpper(const std::string& str) {
    std::string upper_str = str; // 创建一个拷贝
    // 使用 std::transform 和 std::toupper 将字符串中的每个字符转换为大写
    std::transform(upper_str.begin(), upper_str.end(), upper_str.begin(),
                   [](unsigned char c){ return std::toupper(c); }); // 使用 lambda 捕获 c，并确保是 unsigned char 以避免负值问题
    return upper_str;
}
