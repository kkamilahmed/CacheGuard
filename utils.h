#ifndef UTILS_H
#define UTILS_H
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <iostream>

std::string getCurrentTimestamp();
void trimCRLF(std::string &s);
void toUpper(std::string &s);
void importMap(std::unordered_map<std::string, std::string>& store, const std::string& filename) ;
void exportMap(const std::unordered_map<std::string, std::string>& store, const std::string& filename);
void logToFile(const std::string& message);

#endif 
