#include "catch.hpp"
#include "../include/ConsistentHash.h"

TEST_CASE("Consistent Hashing", "[hash]") {
    ConsistentHash hasher;
    hasher.add_node("node1");
    hasher.add_node("node2");
    
    std::string n1 = hasher.get_node("some_key");
    REQUIRE((n1 == "node1" || n1 == "node2"));
}

