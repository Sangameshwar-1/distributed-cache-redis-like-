#include "Config.h"
#include <fstream>
#include <sstream>

Config::Config(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        std::string key, val;
        if (std::getline(iss, key, '=') && std::getline(iss, val)) {
            settings[key] = val;
        }
    }
}

std::string Config::get(const std::string& key, const std::string& default_val) {
    if (settings.find(key) != settings.end()) return settings[key];
    return default_val;
}

int Config::get_int(const std::string& key, int default_val) {
    if (settings.find(key) != settings.end()) return std::stoi(settings[key]);
    return default_val;
}
