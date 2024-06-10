#pragma once
#include <string>
#include <unordered_map>
#include <mutex>

class Cache {
private:
    std::unordered_map<std::string, std::string> store;
    std::mutex mtx;
public:
    void set(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    bool del(const std::string& key);
    bool exists(const std::string& key);
    void clear();
    size_t size();
};

