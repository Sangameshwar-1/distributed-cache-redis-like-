#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../include/Cache.h"

TEST_CASE("SET and GET basic", "[cache][functional]") {
    Cache cache;
    cache.set("name", "Alice");
    REQUIRE(cache.get("name") == "Alice");
}

TEST_CASE("GET unknown key returns empty", "[cache][functional]") {
    Cache cache;
    REQUIRE(cache.get("unknown_key") == "");
}

TEST_CASE("DELETE removes key", "[cache][functional]") {
    Cache cache;
    cache.set("name", "Alice");
    REQUIRE(cache.del("name") == true);
    REQUIRE(cache.get("name") == "");
}

TEST_CASE("DELETE nonexistent key returns false", "[cache][functional]") {
    Cache cache;
    REQUIRE(cache.del("ghost") == false);
}

TEST_CASE("EXISTS returns true for existing key", "[cache][functional]") {
    Cache cache;
    cache.set("x", "1");
    REQUIRE(cache.exists("x") == true);
}

TEST_CASE("EXISTS returns false after delete", "[cache][functional]") {
    Cache cache;
    cache.set("x", "1");
    cache.del("x");
    REQUIRE(cache.exists("x") == false);
}

TEST_CASE("CLEAR removes all keys", "[cache][functional]") {
    Cache cache;
    cache.set("a", "1");
    cache.set("b", "2");
    cache.set("c", "3");
    cache.clear();
    REQUIRE(cache.size() == 0);
    REQUIRE(cache.get("a") == "");
}

TEST_CASE("SIZE returns correct count", "[cache][functional]") {
    Cache cache;
    REQUIRE(cache.size() == 0);
    cache.set("a", "1");
    cache.set("b", "2");
    REQUIRE(cache.size() == 2);
    cache.del("a");
    REQUIRE(cache.size() == 1);
}

TEST_CASE("SET overwrites existing key", "[cache][functional]") {
    Cache cache;
    cache.set("k", "old");
    cache.set("k", "new");
    REQUIRE(cache.get("k") == "new");
    REQUIRE(cache.size() == 1);
}

TEST_CASE("Metrics counters work", "[cache][functional]") {
    Cache cache;
    cache.set("a", "1");
    cache.set("b", "2");
    cache.get("a");
    cache.get("missing");
    REQUIRE(cache.writes >= 2);
    REQUIRE(cache.misses >= 1);
}
