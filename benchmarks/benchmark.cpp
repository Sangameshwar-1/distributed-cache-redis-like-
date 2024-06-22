#include <iostream>
#include <chrono>
#include "../include/Cache.h"

int main() {
    Cache cache;
    auto start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<100000; i++) {
        cache.set("key" + std::to_string(i), "val");
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "100K SETs took " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms\\n";
    return 0;
}

