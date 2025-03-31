#include "admin.h"
#include "utils.h"
#include "wrong.h"
#include <iostream>
#include <fstream>
Admin::Admin() {}
/*不同项用\t分割，vector内部用‘,’分割*/
bool Admin::search(std::string user,std::string db,std::string rightSearch){
    if(user=="root")return true;
    bool res = false;
    std::string file_name = "../../res/rights.rht";
    std::ifstream file(file_name);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_name << std::endl;
        return res;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> info = utils::split(line,"\t");
        std::vector<std::string> rights = utils::split(info[2],",");
        if (info[0]==user && info[1]==db){
            for(std::string right:rights){
                if(right == rightSearch){
                    res = true;
                }
            }
        }
    }
    file.close();
    return res;
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
            Wrong::getInstance("This user don't have such right.")->show();
            //Wrong* wrong = new Wrong("This user don't have such right.");
            //wrong->show();
        }else{
            Wrong::getInstance("Unknown Error")->show();
            //Wrong* wrong = new Wrong("Wrong");
            //wrong->show();
        }
    }
    file.close();
    return res;
}
bool Admin::revoke(std::string user,std::string db,std::vector<std::string> rights){
    return true;
}
