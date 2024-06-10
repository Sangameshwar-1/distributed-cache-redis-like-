#include "Cache.h"

void Cache::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mtx);
    store[key] = value;
}

std::string Cache::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx);
    if (store.find(key) != store.end()) return store[key];
    return ";
}

bool Cache::del(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx);
    return store.erase(key) > 0;
}

bool Cache::exists(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx);
    return store.find(key) != store.end();
}

void Cache::clear() {
    std::lock_guard<std::mutex> lock(mtx);
    store.clear();
}

size_t Cache::size() {
    std::lock_guard<std::mutex> lock(mtx);
    return store.size();
}

