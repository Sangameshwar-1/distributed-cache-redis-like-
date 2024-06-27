#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../include/Cache.h"

TEST_CASE("Cache basic operations", "[cache]") {
    Cache cache;
    cache.set("key1", "val1");
    REQUIRE(cache.get("key1") == "val1");
    REQUIRE(cache.exists("key1"));
    
    cache.del("key1");
    REQUIRE(cache.get("key1") == "");
    REQUIRE(!cache.exists("key1"));
}

TEST_CASE("Cache LRU Eviction", "[cache]") {
    Cache cache;
    for(int i=0; i<1005; ++i) {
        cache.set("k" + std::to_string(i), "v");
    }
    REQUIRE(cache.size() <= 1000);
    REQUIRE(!cache.exists("k0")); // Oldest should be evicted
}
