#pragma once
#include <string>
#include <mutex>

class Logger {
private:
    static std::mutex log_mtx;
    static std::string current_time();
public:
    static void info(const std::string& msg);
    static void warn(const std::string& msg);
    static void error(const std::string& msg);
};

