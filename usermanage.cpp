#include "usermanage.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include "wrong.h"

UserManage::UserManage() {}
void UserManage::createUser(std::string username,std::string password){
    std::ofstream file("../../res/user.txt",std::ios::app);
    if(!file.is_open()){
        std::cerr << "Failed to open file";
        return;
    }
    file<<username<<"\t"<<password<<"\n";
    file.close();
}

void UserManage::dropUser(std::string username){
    std::string filename="../../res/user.txt";
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    std::vector<std::string> lines;
    std::string line;

    // 读取所有行并过滤
    while (std::getline(file, line)) {
        if (line.find(username) == std::string::npos) {
            lines.push_back(line);
        }
    }
    file.close();

    // 重新写回文件
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }

    for (const auto& l : lines) {
        outFile << l << std::endl;
    }
    outFile.close();
}

bool UserManage::findUser(std::string username,std::string password){
    bool res = false;
    std::string filename="../../res/user.txt";
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return res;
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(file, line)) {
        std::vector<std::string> userInfo = utils::split(line,"\t");
        if (userInfo[0]==username && userInfo[1]==password){
            res = true;
        }else if(userInfo[0]==username){
            Wrong* wrong = new Wrong("The password input is wrong for this user.");
            wrong->show();
        }else{
            Wrong* wrong = new Wrong("This user is not exist or not found.");
            wrong->show();
        }
    }
    file.close();
    return res;
}
