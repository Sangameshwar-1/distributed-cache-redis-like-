<p align="center">
  <h1 align="center">⚡ CacheX</h1>
  <p align="center">A high-performance, distributed, in-memory key-value store built from scratch in C++17</p>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/language-C%2B%2B17-blue.svg" alt="C++17">
  <img src="https://img.shields.io/badge/build-CMake-green.svg" alt="CMake">
  <img src="https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg" alt="Platform">
  <img src="https://img.shields.io/badge/tests-passing-brightgreen.svg" alt="Tests">
  <img src="https://img.shields.io/badge/docker-ready-2496ED.svg" alt="Docker">
</p>

---

## What is CacheX?

CacheX is a Redis-inspired distributed caching system designed to explore and implement the core concepts behind modern infrastructure software. It supports **multi-threaded TCP networking**, **LRU eviction**, **TTL-based key expiration**, **data persistence**, **master-replica replication**, **Pub/Sub messaging**, and **consistent hash-based sharding** — all built from first principles without external dependencies.

> **Why this project?** — To deeply understand the internals of systems like Redis, Memcached, and distributed databases by building one from the ground up.

---

## Features

| Category | Feature | Description |
|---|---|---|
| **Core** | In-Memory Store | O(1) `GET`/`SET`/`DEL` via `std::unordered_map` |
| **Core** | LRU Eviction | Doubly-linked list tracks access order; evicts least-recently-used keys when capacity is reached |
| **Core** | TTL Expiration | Background sweeper thread automatically purges expired keys |
| **Concurrency** | Thread Pool | Fixed-size pool of worker threads handles client connections concurrently |
| **Concurrency** | Reader-Writer Locks | `std::shared_mutex` allows concurrent readers while serializing writers |
| **Networking** | TCP Server | Multi-threaded, non-blocking TCP server with proper newline-delimited framing |
| **Networking** | Interactive REPL | Built-in `cachex>` shell connects to servers over TCP |
| **Persistence** | Snapshots | Binary memory dump saved on graceful shutdown (`SIGINT`) |
| **Persistence** | Append-Only File | Every write operation logged to `aof.log` for crash recovery |
| **Distributed** | Master-Replica Sync | Replicas connect to master, receive initial snapshot, then stream live writes |
| **Distributed** | Consistent Hashing | Client-side sharding distributes keys across cluster nodes using a virtual-node hash ring |
| **Messaging** | Pub/Sub | `SUBSCRIBE` / `PUBLISH` for real-time inter-client messaging |
| **Observability** | Metrics | Atomic hit/miss/write counters exposed via `STATS` command |
| **Observability** | Structured Logging | Thread-safe timestamped logging with `INFO`, `WARN`, `ERROR` levels |
| **Config** | File-based Config | `cache.conf` file for port, memory limits, log level, AOF strategy |
| **DevOps** | Docker | Single-command containerized builds |
| **DevOps** | CI/CD | GitHub Actions pipeline for automated builds on push/PR |
| **Testing** | Unit Tests | Catch2-based test suite covering cache operations, eviction, and hash ring distribution |
| **Benchmarks** | Performance | 100K SET operations benchmark included |

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        CacheX Server                            │
│                                                                 │
│  ┌──────────┐    ┌──────────────┐    ┌────────────────────┐    │
│  │  TCP     │───▶│  Thread Pool │───▶│  Command Parser    │    │
│  │  Listener│    │  (4 workers) │    │  SET/GET/DEL/...   │    │
│  └──────────┘    └──────────────┘    └─────────┬──────────┘    │
│                                                │               │
│                                     ┌──────────▼──────────┐    │
│                                     │    Cache Engine      │    │
│                                     │                      │    │
│                                     │  ┌────────────────┐  │    │
│                                     │  │ unordered_map  │  │    │
│                                     │  │   (O(1) ops)   │  │    │
│                                     │  └───────┬────────┘  │    │
│                                     │          │           │    │
│                                     │  ┌───────▼────────┐  │    │
│                                     │  │  LRU List      │  │    │
│                                     │  │ (doubly-linked) │  │    │
│                                     │  └────────────────┘  │    │
│                                     └──────────┬──────────┘    │
│                                                │               │
│                            ┌───────────────────┼──────────┐    │
│                            │                   │          │    │
│                     ┌──────▼─────┐  ┌──────────▼───┐ ┌────▼──┐│
│                     │ Snapshot   │  │  AOF Logger  │ │ TTL   ││
│                     │ (binary)   │  │  (aof.log)   │ │Sweeper││
│                     └────────────┘  └──────────────┘ └───────┘│
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                    Distributed Layer                             │
│                                                                 │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐      │
│  │  Node 6379   │    │  Node 6380   │    │  Node 6381   │      │
│  │  (Master)    │───▶│  (Replica)   │    │  (Replica)   │      │
│  └──────┬───────┘    └──────────────┘    └──────────────┘      │
│         │                                                       │
│  ┌──────▼────────────────────────────────────────────────┐      │
│  │              Consistent Hash Ring                     │      │
│  │   100 virtual nodes per physical node                 │      │
│  │   Keys routed via std::hash → ring.lower_bound()     │      │
│  └───────────────────────────────────────────────────────┘      │
└─────────────────────────────────────────────────────────────────┘
```

---

## Project Structure

```
cachex/
├── include/
│   ├── Cache.h              # Core cache engine with LRU + TTL
│   ├── Server.h             # Multi-threaded TCP server
│   ├── ThreadPool.h         # Fixed-size thread pool
│   ├── ConsistentHash.h     # Hash ring for client-side sharding
│   ├── Logger.h             # Structured logging
│   └── Config.h             # Configuration file parser
├── src/
│   ├── main.cpp             # Entry point, CLI arg parsing, REPL client
│   ├── Cache.cpp            # Cache implementation
│   ├── Server.cpp           # Server implementation with replication
│   ├── Logger.cpp           # Logger implementation
│   └── Config.cpp           # Config parser implementation
├── tests/
│   ├── catch.hpp            # Catch2 single-header test framework
│   ├── test_cache.cpp       # Cache unit tests (ops, LRU eviction)
│   └── test_hash.cpp        # Consistent hashing unit tests
├── benchmarks/
│   └── benchmark.cpp        # Performance benchmark (100K SETs)
├── docker/
│   ├── Dockerfile           # Container build
│   └── docker-compose.yml   # Multi-container orchestration
├── docs/
│   └── Architecture.md      # Architecture documentation
├── .github/workflows/
│   └── ci.yml               # GitHub Actions CI pipeline
├── cache.conf               # Default configuration file
├── CMakeLists.txt           # Build system
├── USAGE.md                 # Detailed usage guide
└── README.md                # This file
```

---

## Quick Start

### Prerequisites

- **C++17** compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **CMake** 3.10+
- **Docker** (optional, for containerized builds)

### Build from Source

```bash
git clone https://github.com/Sangameshwar-1/distributed-cache-redis-like-.git
cd distributed-cache-redis-like-

cmake .
make
```

### Run the Server

```bash
./cachex --server
```

### Run the Client

```bash
./cachex
```

```
cachex> SET name Alice
OK
cachex> GET name
Alice
cachex> SET session abc123 EX 30
OK
cachex> STATS
Hits: 1
Misses: 0
Writes: 2
cachex> exit
```

### Run with Docker

```bash
docker build -t cachex -f docker/Dockerfile .
docker run -p 6379:6379 cachex
```

---

## Supported Commands

| Command | Syntax | Description |
|---|---|---|
| `SET` | `SET <key> <value> [EX <seconds>]` | Store a key-value pair with optional TTL |
| `GET` | `GET <key>` | Retrieve the value for a key |
| `DEL` | `DEL <key>` | Delete a key |
| `STATS` | `STATS` | Show hit/miss/write counters |
| `SUBSCRIBE` | `SUBSCRIBE <channel>` | Subscribe to a Pub/Sub channel |
| `PUBLISH` | `PUBLISH <channel> <message>` | Publish a message to a channel |
| `SYNC` | `SYNC` | (Internal) Trigger replication snapshot |
| `exit` | `exit` | Disconnect from the server |

---

## Configuration

CacheX reads settings from `cache.conf` at startup:

```ini
# Server port
port=6379

# Maximum number of cached entries
max_memory=1024

# Logging level
loglevel=INFO

# AOF sync strategy
aof_sync_strategy=everysec
```

---

## CLI Arguments

| Flag | Description | Example |
|---|---|---|
| `--server` | Start in server mode | `./cachex --server` |
| `--port <N>` | Listen on a specific port | `./cachex --server --port 6380` |
| `--replicaof <host> <port>` | Start as a replica of the given master | `./cachex --server --port 6380 --replicaof 127.0.0.1 6379` |
| `--cluster <ports>` | Connect client to multiple nodes | `./cachex --cluster 6379,6380,6381` |

---

## Testing

Run the Catch2 unit test suite:

```bash
./test_cache
```

```
===============================================================================
All tests passed (7 assertions in 3 test cases)
```

Run the performance benchmark:

```bash
./benchmark
```

```
100K SETs took 2376ms
```

---

## Key Design Decisions

| Decision | Rationale |
|---|---|
| `std::unordered_map` for storage | O(1) average-case lookup, insert, and delete |
| `std::list` for LRU ordering | O(1) splice-to-front on access, O(1) eviction from back |
| `std::shared_mutex` over `std::mutex` | Concurrent reads don't block each other; only writes acquire exclusive locks |
| Background cleaner thread | TTL expiration runs asynchronously to avoid blocking client requests |
| Consistent Hashing with 100 virtual nodes | Minimizes key redistribution when nodes join or leave the cluster |
| Newline-delimited TCP framing | Simple, debuggable protocol; compatible with `telnet` and `nc` |
| Catch2 single-header | Zero-dependency test framework that compiles alongside the project |

---

## License

This project is licensed under the MIT License.
