#include "admin.h"
#include "utils.h"
#include "wrong.h"
#include <iostream>
#include <fstream>
Admin::Admin() {}
bool Admin::search(std::string user,std::string db,std::vector<std::string> right){
    if(user=="root")return true;

    return true;
}
bool Admin::grant(std::string user,std::string db,std::vector<std::string> rights){
    bool res = false;
    std::string filename="../../res/rights.rht";
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return res;
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(file, line)) {
        std::vector<std::string> info = utils::split(line,"\t");
        if (info[0]==user && info[1]==db && info[2]=="wait"){
            res = true;
        }else if(info[0]==user){
            Wrong* wrong = new Wrong("This user don't have such right.");
            wrong->show();
        }else{
            Wrong* wrong = new Wrong("Wrong");
            wrong->show();
        }
    }
    file.close();
    return res;
    return true;
}
bool Admin::revoke(std::string user,std::string db,std::vector<std::string> rights){
    return true;
}
