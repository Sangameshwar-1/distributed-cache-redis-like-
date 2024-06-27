# CacheX: Distributed Redis-like In-Memory Cache

CacheX is a high-performance, distributed, in-memory key-value store built entirely in C++ from the ground up. It implements a multi-threaded architecture mirroring the core design patterns used in enterprise systems like Redis and Memcached.

## 🚀 Features

- **High-Performance Caching:** Built on `std::unordered_map` with an efficient LRU (Least Recently Used) eviction policy.
- **Concurrent Scaling:** Utilizes `std::shared_mutex` (Reader-Writer locks) to ensure high-throughput concurrent `GET` operations while keeping `SET` operations thread-safe.
- **Master-Replica Replication:** Full replication support. Replica nodes connect to the master, request a memory snapshot sync, and listen to real-time `SET`/`DEL` write streams.
- **Client-Side Sharding:** Uses Consistent Hashing (`ConsistentHash.h`) to securely and deterministically partition keys across an array of cluster nodes.
- **Data Persistence:** Graceful shutdowns dump the cache to a `snapshot.bin` binary file. Live mutations are streamed into an Append-Only-File (`aof.log`). The server perfectly reconstructs state on reboot.
- **Interactive REPL:** Built-in TCP networked client providing the interactive `cachex>` shell for cluster management.
- **Production-Ready Tools:** Structured logging, `.conf` file parsing, and rigorous unit testing via Catch2.

## 🛠️ Architecture

The project was developed in 13 structural phases:
1. Core Cache Engine (`Cache.h`)
2. CLI and REPL 
3. Multi-threaded TCP Server (`ThreadPool.h`)
4. TTL & Background Expiration Sweeping
5. LRU Eviction Policies
6. Persistence (Snapshot + AOF)
7. Live Master-Replica Sync
8. Publish / Subscribe Architecture
9. Consistent Hashing
10. Containerization (Docker)
11. CI/CD (GitHub Actions)

## 📦 Getting Started

### Prerequisites
- C++17 or higher
- CMake (3.10+)

### Building
```bash
cmake .
make
```

### Running the Server
```bash
./cachex --server --port 6379
```

### Running a Replica
```bash
./cachex --server --port 6380 --replicaof 127.0.0.1 6379
```

### Running the Interactive Client
Pass a comma-separated list of nodes to the client to utilize the Consistent Hashing router:
```bash
./cachex --cluster 6379,6380
```

```text
CacheX REPL Client Started. Ports: 6379 6380 
cachex> SET mykey 123
OK
cachex> GET mykey
123
```

### Running the Tests
```bash
./test_cache
```

## 📜 License
MIT License
