#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <iomanip>
#include "../include/Cache.h"

int main() {
    Cache cache;
    const int NUM_OPS = 100000;

    std::cout << "=== CacheX In-Memory Benchmark ===" << std::endl;
    std::cout << "Operations: " << NUM_OPS << std::endl << std::endl;

    // ---------- Write Benchmark ----------
    auto write_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_OPS; i++) {
        cache.set("key" + std::to_string(i), "value" + std::to_string(i));
    }
    auto write_end = std::chrono::high_resolution_clock::now();
    double write_ms = std::chrono::duration_cast<std::chrono::microseconds>(write_end - write_start).count() / 1000.0;
    double write_ops_sec = NUM_OPS / (write_ms / 1000.0);
    double write_latency_us = (write_ms * 1000.0) / NUM_OPS;

    std::cout << "[WRITE]" << std::endl;
    std::cout << "  Total time:     " << std::fixed << std::setprecision(2) << write_ms << " ms" << std::endl;
    std::cout << "  Throughput:     " << std::fixed << std::setprecision(0) << write_ops_sec << " ops/sec" << std::endl;
    std::cout << "  Avg latency:    " << std::fixed << std::setprecision(2) << write_latency_us << " us" << std::endl;
    std::cout << std::endl;

    // ---------- Read Benchmark ----------
    auto read_start = std::chrono::high_resolution_clock::now();
    int hit = 0, miss = 0;
    for (int i = 0; i < NUM_OPS; i++) {
        std::string val = cache.get("key" + std::to_string(i));
        if (val.empty()) miss++;
        else hit++;
    }
    auto read_end = std::chrono::high_resolution_clock::now();
    double read_ms = std::chrono::duration_cast<std::chrono::microseconds>(read_end - read_start).count() / 1000.0;
    double read_ops_sec = NUM_OPS / (read_ms / 1000.0);
    double read_latency_us = (read_ms * 1000.0) / NUM_OPS;

    std::cout << "[READ]" << std::endl;
    std::cout << "  Total time:     " << std::fixed << std::setprecision(2) << read_ms << " ms" << std::endl;
    std::cout << "  Throughput:     " << std::fixed << std::setprecision(0) << read_ops_sec << " ops/sec" << std::endl;
    std::cout << "  Avg latency:    " << std::fixed << std::setprecision(2) << read_latency_us << " us" << std::endl;
    std::cout << "  Hit rate:       " << std::fixed << std::setprecision(1) << (100.0 * hit / NUM_OPS) << "%" << std::endl;
    std::cout << std::endl;

    // ---------- Mixed Workload ----------
    auto mixed_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_OPS; i++) {
        if (i % 3 == 0) {
            cache.set("mix" + std::to_string(i), "val");
        } else {
            cache.get("mix" + std::to_string(i));
        }
    }
    auto mixed_end = std::chrono::high_resolution_clock::now();
    double mixed_ms = std::chrono::duration_cast<std::chrono::microseconds>(mixed_end - mixed_start).count() / 1000.0;
    double mixed_ops_sec = NUM_OPS / (mixed_ms / 1000.0);

    std::cout << "[MIXED 33% write / 67% read]" << std::endl;
    std::cout << "  Total time:     " << std::fixed << std::setprecision(2) << mixed_ms << " ms" << std::endl;
    std::cout << "  Throughput:     " << std::fixed << std::setprecision(0) << mixed_ops_sec << " ops/sec" << std::endl;
    std::cout << std::endl;

    // ---------- Summary ----------
    std::cout << "=== Summary ===" << std::endl;
    std::cout << "  Cache size:     " << cache.size() << " entries" << std::endl;
    std::cout << "  Total writes:   " << cache.writes.load() << std::endl;
    std::cout << "  Total hits:     " << cache.hits.load() << std::endl;
    std::cout << "  Total misses:   " << cache.misses.load() << std::endl;

    // ---------- CSV Output ----------
    std::ofstream csv("benchmarks/results.csv");
    csv << "metric,value" << std::endl;
    csv << "write_ops_sec," << std::fixed << std::setprecision(0) << write_ops_sec << std::endl;
    csv << "read_ops_sec," << std::fixed << std::setprecision(0) << read_ops_sec << std::endl;
    csv << "mixed_ops_sec," << std::fixed << std::setprecision(0) << mixed_ops_sec << std::endl;
    csv << "write_latency_us," << std::fixed << std::setprecision(2) << write_latency_us << std::endl;
    csv << "read_latency_us," << std::fixed << std::setprecision(2) << read_latency_us << std::endl;
    csv << "write_total_ms," << std::fixed << std::setprecision(2) << write_ms << std::endl;
    csv << "read_total_ms," << std::fixed << std::setprecision(2) << read_ms << std::endl;
    csv << "cache_size," << cache.size() << std::endl;
    csv << "hit_rate_pct," << std::fixed << std::setprecision(1) << (100.0 * hit / NUM_OPS) << std::endl;
    csv.close();

    std::cout << "\nResults saved to benchmarks/results.csv" << std::endl;
    return 0;
}
