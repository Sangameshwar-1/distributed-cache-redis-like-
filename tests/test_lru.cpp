#include "catch.hpp"
#include "../include/Cache.h"

TEST_CASE("LRU eviction removes oldest key", "[cache][lru]") {
    Cache cache;
    for (int i = 0; i < 1005; ++i) {
        cache.set("k" + std::to_string(i), "v" + std::to_string(i));
    }
    REQUIRE(cache.size() <= 1000);
    REQUIRE(cache.exists("k0") == false);
    REQUIRE(cache.exists("k1") == false);
    REQUIRE(cache.exists("k2") == false);
    REQUIRE(cache.exists("k3") == false);
    REQUIRE(cache.exists("k4") == false);
    REQUIRE(cache.exists("k1004") == true);
    REQUIRE(cache.exists("k1000") == true);
}

TEST_CASE("LRU promotes accessed keys", "[cache][lru]") {
    Cache cache;
    for (int i = 0; i < 1000; ++i) {
        cache.set("k" + std::to_string(i), "v");
    }
    cache.get("k0");
    cache.set("knew", "vnew");
    REQUIRE(cache.exists("k0") == true);
    REQUIRE(cache.exists("k1") == false);
}

TEST_CASE("SET on existing key does not increase size", "[cache][lru]") {
    Cache cache;
    for (int i = 0; i < 500; ++i) {
        cache.set("k" + std::to_string(i), "v");
    }
    size_t before = cache.size();
    cache.set("k0", "updated");
    REQUIRE(cache.size() == before);
}
