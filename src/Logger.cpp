#include "Logger.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

std::mutex Logger::log_mtx;

std::string Logger::current_time() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
#ifdef _WIN32
    struct tm timeinfo;
    localtime_s(&timeinfo, &now_time);
    ss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
#else
    ss << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S");
#endif
    return ss.str();
}

void Logger::info(const std::string& msg) {
    std::lock_guard<std::mutex> lock(log_mtx);
    std::cout << "[" << current_time() << "] [INFO] " << msg << std::endl;
}

void Logger::warn(const std::string& msg) {
    std::lock_guard<std::mutex> lock(log_mtx);
    std::cout << "[" << current_time() << "] [WARN] " << msg << std::endl;
}

void Logger::error(const std::string& msg) {
    std::lock_guard<std::mutex> lock(log_mtx);
    std::cerr << "[" << current_time() << "] [ERROR] " << msg << std::endl;
}

