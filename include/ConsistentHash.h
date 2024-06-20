#pragma once
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <algorithm>

class ConsistentHash {
private:
    std::map<size_t, std::string> ring;
    int virtual_nodes;
    std::hash<std::string> hasher;
public:
    ConsistentHash(int v_nodes = 100) : virtual_nodes(v_nodes) {}
    void add_node(const std::string& node) {
        for (int i = 0; i < virtual_nodes; ++i) {
            size_t hash = hasher(node + "#" + std::to_string(i));
            ring[hash] = node;
        }
    }
    void remove_node(const std::string& node) {
        for (int i = 0; i < virtual_nodes; ++i) {
            size_t hash = hasher(node + "#" + std::to_string(i));
            ring.erase(hash);
        }
    }
    std::string get_node(const std::string& key) {
        if (ring.empty()) return ";
        size_t hash = hasher(key);
        auto it = ring.lower_bound(hash);
        if (it == ring.end()) it = ring.begin();
        return it->second;
    }
};

