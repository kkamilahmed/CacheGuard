#include "executor.h"
#include <sstream>
#include <algorithm>
#include "utils.h"


std::string Executor::execute(const std::string& cmdLine) {
    std::string line = cmdLine;
    trimCRLF(line);
    if (line.empty()) return "";

    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;
    toUpper(cmd);

    if (cmd == "PING") {
        return "+PONG\r\n";
    }
    else if (cmd == "SET") {
        std::string key, value;
        iss >> key >> std::ws; 
        std::getline(iss, value);
         size_t pos = value.find(" EX ");
        if (pos != std::string::npos) {
            std::string ttl_str = value.substr(pos + 4); 
            value = value.substr(0, pos);               
            int ttl = std::stoi(ttl_str); 
            storage_.set(key, value, ttl);
        }

        else{
            storage_.set(key, value,0);
        }


        return "+OK\r\n";
    }
    else if (cmd == "GET") {
        std::string key;
        iss >> key;
        std::string value;
        if (storage_.get(key, value)) {
            return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
        } else {
            return "$-1\r\n";
        }
    }
    else if (cmd=="SHOW"){
        storage_.show();
        return "Done\r\n";
    }
    else if (cmd=="SAVE"){
        storage_.save();
        return "Done\r\n";
    }
    else {
        return "-ERROR unknown command\r\n";
    }
}
