#include "admin.h"
#include "utils.h"
#include "wrong.h"
#include <iostream>
#include <fstream>
Admin::Admin() {}
/**对right的存储形式的说明
 * [user]\t[db]\t[right](select/insert/delete/update)\n
 * */
/*不同项用\t分割，vector内部用‘,’分割*/
bool Admin::search(std::string &userSearch, std::string &dbSearch, std::string &rightSearch) {
    if (userSearch == "root") return true;

    std::ifstream file("../../res/rights.rht");
    if (!file.is_open()) {
        std::cerr << "Failed to open file: ../../res/rights.rht" << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> info = utils::split(line, "\t");
        if (info.size() < 3) continue;

        if (info[0] == userSearch && info[1] == dbSearch && info[2] == rightSearch) {
            return true;
        }
    }

    return false;
}

bool Admin::grant(std::string &userSearch, std::string &dbSearch, std::string &rightSearch) {
    std::string filename = "../../res/rights.rht";
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> info = utils::split(line, "\t");
        if (info.size() < 3) continue;  // 防止格式错误

        if (info[0] == userSearch && info[1] == dbSearch && info[2] == rightSearch) {
            file.close();
            return true;  // 权限已存在
        }
    }
    file.close(); // 关闭输入流

    // 追加模式打开文件
    std::ofstream file2(filename, std::ios::app);
    if (!file2.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }
    file2 << userSearch << "\t" << dbSearch << "\t" << rightSearch << "\n";
    file2.close();

    return true;
}

bool Admin::revoke(std::string & userSearch, std::string & dbSearch, std::string & rightSearch) {
    bool res = false;
    std::string filename = "../../res/rights.rht";
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return res;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> info = utils::split(line, "\t");
        if (info.size() < 3) continue; // 防止格式错误

        std::string user = info[0];
        std::string db = info[1];
        std::string right = info[2];

        if (user == userSearch && db == dbSearch && right == rightSearch) {
            res = true; // 找到匹配项
        } else {
            lines.push_back(line); // 其他行保留
        }
    }
    file.close();

    if (res) {
        std::ofstream outFile(filename, std::ios::trunc);
        if (!outFile.is_open()) {
            std::cerr << "Failed to write to file: " << filename << std::endl;
            return false;
        }
        for (const std::string &savedLine : lines) {
            outFile << savedLine << "\n";
        }
        outFile.close();
    }

    return res;
}

