#include "parser.h"

std::vector<std::string> Parser::parseCommands(std::string &buffer) {
    std::vector<std::string> commands;
    size_t pos;
    while ((pos = buffer.find("\n")) != std::string::npos) {
        std::string line = buffer.substr(0, pos); 
        buffer.erase(0, pos + 1);

        if (!line.empty() && line.back() == '\r') line.pop_back();

        if (!line.empty()) commands.push_back(line);
    }
    return commands;
}
