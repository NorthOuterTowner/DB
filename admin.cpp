#include "admin.h"
#include "logger.h"
#include "utils.h"
#include "wrong.h"
#include <iostream>
#include <fstream>

static Logger logger("../../res/system_logs.txt");

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
    // 检查是否为root用户
    if (userSearch == "root") {
        logger.log("admin", "GRANT", "permission", "Attempt to grant permission to root user");
        return false; // root用户不允许被授予权限
    }

    std::string filename = "../../res/rights.rht";
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        logger.log("admin", "GRANT", "permission", "Failed to open file for reading: " + filename);
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> info = utils::split(line, "\t");
        if (info.size() < 3) continue;  // 防止格式错误

        if (info[0] == userSearch && info[1] == dbSearch && info[2] == rightSearch) {
            file.close();
            logger.log("admin", "GRANT", "permission", "Permission already exists for user: " + userSearch);
            return true;  // 权限已存在
        }
    }
    file.close(); // 关闭输入流

    // 追加模式打开文件
    std::ofstream file2(filename, std::ios::app);
    if (!file2.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        logger.log("admin", "GRANT", "permission", "Failed to open file for granting permission: " + filename);
        return false;
    }
    file2 << userSearch << "\t" << dbSearch << "\t" << rightSearch << "\n";
    file2.close();

    // 记录权限变更
    std::string logMessage = "Granted " + rightSearch + " on database " + dbSearch + " to user " + userSearch;
    logger.log("admin", "GRANT", "permission", logMessage); // 记录管理员权限操作

    return true;
}

bool Admin::revoke(std::string & userSearch, std::string & dbSearch, std::string & rightSearch) {
    bool res = false;
    std::string filename = "../../res/rights.rht";
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        logger.log("admin", "REVOKE", "permission", "Failed to open file for reading: " + filename);
        return res;
    }

    std::vector<std::string> lines; // 保存读取的行
    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> info = utils::split(line, "\t");
        if (info.size() < 3) continue; // 防止格式错误

        std::string user = info[0];
        std::string db = info[1];
        std::string right = info[2];

        // 检查是否匹配到用户和权限
        if (user == userSearch && db == dbSearch && right == rightSearch) {
            res = true; // 找到匹配项
        } else {
            lines.push_back(line); // 其他行保留
        }
    }
    file.close(); // 关闭输入流

    if (res) {
        // 重新打开文件，准备写入更新后的权限列表
        std::ofstream outFile(filename, std::ios::trunc);
        if (!outFile.is_open()) {
            std::cerr << "Failed to write to file: " << filename << std::endl;
            logger.log("admin", "REVOKE", "permission", "Failed to open file for writing: " + filename);
            return false;
        }

        // 写回未被撤销的权限行
        for (const std::string &savedLine : lines) {
            outFile << savedLine << "\n";
        }
        outFile.close();

        // 记录权限被撤销的操作
        std::string logMessage = "Revoked " + rightSearch + " on database " + dbSearch + " from user " + userSearch;
        logger.log("admin", "REVOKE", "permission", logMessage); // 记录管理员权限操作
    } else {
        logger.log("admin", "REVOKE", "permission", "No matching permission found for user: " + userSearch); // 记录未找到权限的信息
    }

    return res;
}
