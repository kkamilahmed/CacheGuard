#include <string>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <iostream>
 








void trimCRLF(std::string &s) {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
}

void toUpper(std::string &s) {
    for (auto &c : s) c = toupper(c);
 }

void importMap(std::unordered_map<std::string, std::string>& store, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file for reading\n";
        return;
    }

    store.clear();
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string key, value;
        if (std::getline(ss, key, '\t') && std::getline(ss, value)) {
            store[key] = value;
        }
    }

    file.close();
}

void exportMap(const std::unordered_map<std::string, std::string>& store, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file for writing\n";
        return;
    }

    for (const auto& [key, value] : store) {
        file << key << "\t" << value << "\n"; 
    }

    file.close();
}