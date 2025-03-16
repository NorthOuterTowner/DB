#include "utils.h"
#include <vector>
#include <string>

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
