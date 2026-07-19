#include "catch.hpp"
#include "../include/Cache.h"
#include <cstdio>

TEST_CASE("Snapshot save and load restores data", "[cache][persistence]") {
    const std::string snapfile = "test_snapshot.bin";
    {
        Cache cache;
        cache.set("fruit", "apple");
        cache.set("color", "red");
        cache.set("number", "42");
        cache.snapshot(snapfile);
    }
    {
        Cache cache;
        cache.load_snapshot(snapfile);
        REQUIRE(cache.get("fruit") == "apple");
        REQUIRE(cache.get("color") == "red");
        REQUIRE(cache.get("number") == "42");
    }
    std::remove(snapfile.c_str());
}

TEST_CASE("Snapshot preserves correct count", "[cache][persistence]") {
    const std::string snapfile = "test_snapshot2.bin";
    {
        Cache cache;
        for (int i = 0; i < 100; ++i) {
            cache.set("k" + std::to_string(i), "v" + std::to_string(i));
        }
        cache.snapshot(snapfile);
    }
    {
        Cache cache;
        cache.load_snapshot(snapfile);
        REQUIRE(cache.size() == 100);
    }
    std::remove(snapfile.c_str());
}

TEST_CASE("Empty snapshot loads without error", "[cache][persistence]") {
    const std::string snapfile = "test_empty_snapshot.bin";
    {
        Cache cache;
        cache.snapshot(snapfile);
    }
    {
        Cache cache;
        cache.load_snapshot(snapfile);
        REQUIRE(cache.size() == 0);
    }
    std::remove(snapfile.c_str());
}
