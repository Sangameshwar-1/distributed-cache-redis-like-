#include "catch.hpp"
#include "../include/ConsistentHash.h"
#include <map>

TEST_CASE("Hash ring returns a valid node", "[hash]") {
    ConsistentHash hasher;
    hasher.add_node("node1");
    hasher.add_node("node2");
    std::string n = hasher.get_node("some_key");
    REQUIRE((n == "node1" || n == "node2"));
}

TEST_CASE("Same key always maps to same node", "[hash]") {
    ConsistentHash hasher;
    hasher.add_node("node1");
    hasher.add_node("node2");
    hasher.add_node("node3");
    std::string first = hasher.get_node("stable_key");
    for (int i = 0; i < 100; ++i) {
        REQUIRE(hasher.get_node("stable_key") == first);
    }
}

TEST_CASE("Empty ring returns empty string", "[hash]") {
    ConsistentHash hasher;
    REQUIRE(hasher.get_node("any_key") == "");
}

TEST_CASE("Remove node changes routing", "[hash]") {
    ConsistentHash hasher;
    hasher.add_node("A");
    hasher.add_node("B");
    std::string before = hasher.get_node("test_key");
    hasher.remove_node(before);
    std::string after = hasher.get_node("test_key");
    REQUIRE(after != before);
}

TEST_CASE("10000 keys distribute roughly balanced across 3 nodes", "[hash][sharding]") {
    ConsistentHash hasher;
    hasher.add_node("127.0.0.1:6379");
    hasher.add_node("127.0.0.1:6380");
    hasher.add_node("127.0.0.1:6381");
    std::map<std::string, int> distribution;
    for (int i = 0; i < 10000; ++i) {
        std::string node = hasher.get_node("key" + std::to_string(i));
        distribution[node]++;
    }
    REQUIRE(distribution.size() == 3);
    for (auto& p : distribution) {
        REQUIRE(p.second > 2000);
    }
}
