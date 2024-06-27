#pragma once
#include <string>
#include <unordered_map>

class Config {
private:
    std::unordered_map<std::string, std::string> settings;
public:
    Config(const std::string& filename);
    std::string get(const std::string& key, const std::string& default_val);
    int get_int(const std::string& key, int default_val);
};

