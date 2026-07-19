#include "catch.hpp"
#include "../include/Cache.h"
#include <thread>
#include <chrono>

TEST_CASE("TTL key accessible before expiry", "[cache][ttl]") {
    Cache cache;
    cache.set("otp", "123", 3);
    REQUIRE(cache.get("otp") == "123");
    REQUIRE(cache.exists("otp") == true);
}

TEST_CASE("TTL key expires after duration", "[cache][ttl]") {
    Cache cache;
    cache.set("otp", "123", 2);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    REQUIRE(cache.get("otp") == "");
}

TEST_CASE("Key without TTL never expires", "[cache][ttl]") {
    Cache cache;
    cache.set("permanent", "value");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    REQUIRE(cache.get("permanent") == "value");
}

TEST_CASE("EXISTS returns false for expired key", "[cache][ttl]") {
    Cache cache;
    cache.set("temp", "val", 1);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    REQUIRE(cache.exists("temp") == false);
}
