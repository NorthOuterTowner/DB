#ifndef UTILS_H
#define UTILS_H
#include <vector>
#include <string>

class utils
{
public:
    utils();
    static std::vector<std::string> split(const std::string& str, const std::string& delimiter);
    static std::string strip(const std::string& str);
    static std::string toUpper(const std::string& str);
};

#endif // UTILS_H
