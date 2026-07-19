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
  <img src="https://img.shields.io/badge/flask-dashboard-orange.svg" alt="Flask Dashboard">
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
| **Observability** | Metrics Dashboard | **NEW!** Real-time Flask UI to monitor hit rates, memory, and throughput |
| **Observability** | Structured Logging | Thread-safe timestamped logging with `INFO`, `WARN`, `ERROR` levels |
| **Config** | File-based Config | `cache.conf` file for port, memory limits, log level, AOF strategy |
| **Testing** | 17-Level Test Suite | Comprehensive Catch2 and Python automated testing (functional, load, stress) |

---

## Architecture Deep-Dive

We've prepared a comprehensive technical deep-dive into the internals of CacheX. 
👉 [**Read the Architecture Documentation**](Architecture.md) to learn how the Thread Pool, LRU Eviction, and Consistent Hashing algorithms are implemented.

---

## Quick Start

### Prerequisites

- **C++17** compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **CMake** 3.10+
- **Docker** (optional)
- **Python 3** (for the UI Dashboard)

### Build from Source

```bash
cmake .
make
```

### Run the Server

```bash
./cachex --server
```

### Run the Client REPL

```bash
./cachex
```

```
cachex> SET event_123 "base64_data" EX 300
OK
cachex> GET event_123
"base64_data"
cachex> STATS
Hits: 1
Misses: 0
Writes: 1
```

### Run the Flask Performance Dashboard

We've built a sleek, real-time UI dashboard to monitor CacheX operations (perfect for visualizing MERN stack traffic).

1. Install dependencies:
```bash
pip install -r dashboard/requirements.txt
```
2. Start the dashboard:
```bash
python dashboard/app.py
```
3. Open `http://localhost:5050` in your browser.

---

## Docker Integration

CacheX is fully containerized and easily drops into existing projects.

```bash
docker build -t cachex -f docker/Dockerfile .
docker run -p 6379:6379 cachex
```

In your `docker-compose.yml`:
```yaml
services:
  cachex:
    image: cachex
    ports:
      - "6379:6379"
```

---

## Supported Commands

| Command | Syntax | Description |
|---|---|---|
| `SET` | `SET <key> <value> [EX <seconds>]` | Store a key-value pair with optional TTL |
| `GET` | `GET <key>` | Retrieve the value for a key |
| `DEL` | `DEL <key>` | Delete a key |
| `EXISTS` | `EXISTS <key>` | Check if a key is in memory |
| `CLEAR` | `CLEAR` | Evict all keys |
| `SIZE` | `SIZE` | Get total number of stored keys |
| `STATS` | `STATS` | Show hit/miss/write counters |
| `SUBSCRIBE` | `SUBSCRIBE <channel>` | Subscribe to a Pub/Sub channel |
| `PUBLISH` | `PUBLISH <channel> <message>` | Publish a message to a channel |

---

## Testing

Run the Catch2 unit test suite:

```bash
./test_cache
```

Run the performance benchmark (100,000 SET operations):

```bash
./benchmark
```

---

## License

This project is licensed under the MIT License.
