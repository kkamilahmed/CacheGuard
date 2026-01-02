#ifndef STORAGE_H
#define STORAGE_H

#include <string>

class Storage {
public:
    void set(const std::string &key, const std::string &value,int time);
    bool get(const std::string &key, std::string&value);
    void show();
    void save();
    void load();
};

#endif // STORAGE_H
