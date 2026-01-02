#include "storage.h"
#include <unordered_map>
#include <mutex>
#include <iostream>
#include "utils.h"
#include <thread>
#include <iostream>
#include <chrono>

static std::unordered_map<std::string, std::string> store;
static std::mutex store_mutex;

void Storage::load() {
    std::lock_guard<std::mutex> lock(store_mutex);
    
    std::ifstream file("dump.txt");
    if (file.is_open()) {
        file.close();  
        importMap(store, "dump.txt");
        std::cout << "Loaded store from dump.txt\n";
    } else {
        std::cout << "No dump.txt found, starting with empty store\n";
    }
}




void Storage::set(const std::string& key, const std::string& value, int ttl_seconds) {
    {
        std::lock_guard<std::mutex> lock(store_mutex);
        store[key] = value;
    }

    if (ttl_seconds > 0) {

        std::thread([this, key, ttl_seconds]() {
            std::this_thread::sleep_for(std::chrono::seconds(ttl_seconds));
            std::lock_guard<std::mutex> lock(store_mutex);
            store.erase(key);
            std::cout << "deadass?";
        }).detach();
    }
}

bool Storage::get(const std::string& key, std::string& value) {
    std::lock_guard<std::mutex> lock(store_mutex);
    auto it = store.find(key);
    if (it != store.end()) {
        value = it->second;
        return true;
    }
    return false;
}

void Storage::show(){
    std::lock_guard<std::mutex> lock(store_mutex);
    for (const auto& pair : store) {
    std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
    }
}


void Storage::save(){
    std::lock_guard<std::mutex> lock(store_mutex);
    exportMap(store, "dump.txt");
}