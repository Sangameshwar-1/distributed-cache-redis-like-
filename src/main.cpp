#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "Cache.h"

int main() {
    Cache cache;
    std::string line;
    std::cout << "CacheX REPL (Phase 2)
";
    while (true) {
        std::cout << "cachex> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") break;
        
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        if (cmd == "SET") {
            std::string key, val;
            iss >> key >> val;
            cache.set(key, val);
            std::cout << "OK
";
        } else if (cmd == "GET") {
            std::string key;
            iss >> key;
            std::string val = cache.get(key);
            if (val.empty()) std::cout << "(nil)
";
            else std::cout << val << "\\n";
        } else if (cmd == "DELETE") {
            std::string key;
            iss >> key;
            if (cache.del(key)) std::cout << "OK
";
            else std::cout << "(nil)
";
        } else if (cmd == "EXISTS") {
            std::string key;
            iss >> key;
            std::cout << (cache.exists(key) ? "1
" : "0
");
        } else if (cmd == "CLEAR") {
            cache.clear();
            std::cout << "OK
";
        } else if (cmd == "SIZE") {
            std::cout << cache.size() << "\\n";
        } else {
            std::cout << "Unknown command
";
        }
    }
    return 0;
}

