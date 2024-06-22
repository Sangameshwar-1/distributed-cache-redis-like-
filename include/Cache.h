#pragma once
#include <string>
#include <unordered_map>
#include <list>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <fstream>

struct CacheItem {
    std::string key;
    std::string value;
    long long expiry;
};

class Cache {
private:
    size_t capacity = 1000;
    std::list<CacheItem> lru_list;
    std::unordered_map<std::string, std::list<CacheItem>::iterator> store;
    std::mutex mtx;
    std::thread cleaner_thread;
    bool stop_cleaner;
    void evict();
    void cleaner_loop();
public:
    void snapshot(const std::string& filename);
    void load_snapshot(const std::string& filename);
    void append_aof(const std::string& cmd);

    Cache();
    ~Cache();
    void set(const std::string& key, const std::string& value, int ttl_sec = 0);
    std::string get(const std::string& key);
    bool del(const std::string& key);
    bool exists(const std::string& key);
    void clear();
    size_t size();
};



