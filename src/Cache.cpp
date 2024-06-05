#include "Cache.h"

void Cache::set(const std::string& key, const std::string& value) {
    store[key] = value;
}

std::string Cache::get(const std::string& key) {
    if (store.find(key) != store.end()) return store[key];
    return ";
}

bool Cache::del(const std::string& key) {
    return store.erase(key) > 0;
}

bool Cache::exists(const std::string& key) {
    return store.find(key) != store.end();
}

void Cache::clear() {
    store.clear();
}

size_t Cache::size() {
    return store.size();
}

