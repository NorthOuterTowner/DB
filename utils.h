#ifndef UTILS_H
#define UTILS_H
#include <vector>
#include <string>

class utils
{
public:
    utils();
    static std::vector<std::string> split(const std::string& str, const std::string& delimiter);
};

#endif // UTILS_H
