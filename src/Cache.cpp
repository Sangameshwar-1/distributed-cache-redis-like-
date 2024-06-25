#include "Cache.h"

Cache::Cache() : stop_cleaner(false) {
    cleaner_thread = std::thread(&Cache::cleaner_loop, this);
}
Cache::~Cache() {
    stop_cleaner = true;
    if (cleaner_thread.joinable()) cleaner_thread.join();
}
void Cache::cleaner_loop() {
    while (!stop_cleaner) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::lock_guard<std::mutex> lock(mtx);
        auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        for (auto it = lru_list.begin(); it != lru_list.end(); ) {
            if (it->expiry > 0 && it->expiry < now) {
                store.erase(it->key);
                it = lru_list.erase(it);
            } else {
                ++it;
            }
        }
    }
}
void Cache::evict() {
    if (lru_list.empty()) return;
    auto last = lru_list.back();
    store.erase(last.key);
    lru_list.pop_back();
}
void Cache::set(const std::string& key, const std::string& value, int ttl_sec) {
    std::lock_guard<std::mutex> lock(mtx);
    auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    long long expiry = ttl_sec > 0 ? now + ttl_sec : 0;
    if (store.find(key) != store.end()) {
        lru_list.erase(store[key]);
    } else if (store.size() >= capacity) {
        evict();
    }
    lru_list.push_front({key, value, expiry});
    store[key] = lru_list.begin();
    writes++;
    append_aof("SET " + key + " " + value);

}
std::string Cache::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx);
    if (store.find(key) == store.end()) { misses++; return ""; }
    auto it = store[key];
    auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (it->expiry > 0 && it->expiry < now) {
        store.erase(key);
        lru_list.erase(it);
        return "";
    }
    lru_list.splice(lru_list.begin(), lru_list, it);
    return it->value;
}
bool Cache::del(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx);
    if (store.find(key) == store.end()) return false;
    lru_list.erase(store[key]);
    store.erase(key);
    return true;
}
bool Cache::exists(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx);
    if (store.find(key) == store.end()) return false;
    auto it = store[key];
    auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (it->expiry > 0 && it->expiry < now) return false;
    return true;
}
void Cache::clear() {
    std::lock_guard<std::mutex> lock(mtx);
    store.clear();
    lru_list.clear();
}
size_t Cache::size() {
    std::lock_guard<std::mutex> lock(mtx);
    return store.size();
}


void Cache::snapshot(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mtx);
    std::ofstream out(filename, std::ios::binary);
    for (const auto& item : lru_list) {
        out << item.key << " " << item.value << " " << item.expiry << "\\n";
    }
}

void Cache::load_snapshot(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mtx);
    std::ifstream in(filename, std::ios::binary);
    std::string k, v; long long exp;
    while (in >> k >> v >> exp) {
        lru_list.push_front({k, v, exp});
        store[k] = lru_list.begin();
    }
}

void Cache::append_aof(const std::string& cmd) {
    std::ofstream out("aof.log", std::ios::app);
    out << cmd << "\\n";
}


