# CacheX Usage Guide

This guide walks through every feature of CacheX with concrete examples. For installation and build instructions, see the [README](README.md).

---

## Table of Contents

- [Starting a Server](#1-starting-a-server)
- [Using the Interactive Shell](#2-using-the-interactive-shell)
- [Key-Value Operations](#3-key-value-operations)
- [TTL (Time-To-Live)](#4-ttl-time-to-live)
- [LRU Eviction](#5-lru-eviction)
- [Pub/Sub Messaging](#6-pubsub-messaging)
- [Server Metrics](#7-server-metrics)
- [Master-Replica Replication](#8-master-replica-replication)
- [Cluster Sharding](#9-cluster-sharding-consistent-hashing)
- [Persistence & Recovery](#10-persistence--recovery)
- [Configuration](#11-configuration)
- [Docker Deployment](#12-docker-deployment)
- [Running Tests & Benchmarks](#13-running-tests--benchmarks)

---

## 1. Starting a Server

Start a CacheX server on the default port (6379):

```bash
./cachex --server
```

Output:
```
[2024-06-27 10:00:00] [INFO] Loading snapshot...
[2024-06-27 10:00:00] [INFO] Replaying AOF...
[2024-06-27 10:00:00] [INFO] Server listening on port 6379
```

To start on a custom port:

```bash
./cachex --server --port 7000
```

---

## 2. Using the Interactive Shell

Run `cachex` without `--server` to launch the built-in REPL client:

```bash
./cachex
```

```
CacheX REPL Client Started. Ports: 6379
cachex> _
```

The client opens a TCP connection to the server, sends your command, prints the response, and waits for the next input. Type `exit` or `quit` to disconnect.

---

## 3. Key-Value Operations

### SET — Store a value

```
cachex> SET user:1001 "Alice"
OK
```

### GET — Retrieve a value

```
cachex> GET user:1001
Alice
```

### GET — Key does not exist

```
cachex> GET user:9999
(nil)
```

### DEL — Delete a key

```
cachex> DEL user:1001
OK
cachex> GET user:1001
(nil)
```

---

## 4. TTL (Time-To-Live)

Set a key that automatically expires after a given number of seconds using the `EX` flag:

```
cachex> SET session:abc token_xyz EX 30
OK
```

This key will be accessible for 30 seconds. After that, a background sweeper thread removes it automatically:

```
cachex> GET session:abc      # within 30 seconds
token_xyz

cachex> GET session:abc      # after 30 seconds
(nil)
```

**How it works internally:**
- Each `CacheItem` stores an `expiry` timestamp (epoch seconds).
- A dedicated background thread (`cleaner_loop`) wakes up every second and scans the LRU list, removing any items whose expiry has passed.

---

## 5. LRU Eviction

CacheX enforces a maximum capacity of **1000 entries** by default. When the store is full and a new key is inserted, the **Least Recently Used** key is evicted.

```
cachex> SET k0 v0
OK
cachex> SET k1 v1
OK
...
cachex> SET k1004 v1004     # This triggers eviction of k0, k1, k2, k3, k4
OK
cachex> GET k0
(nil)                        # Evicted
```

**How it works internally:**
- A `std::list<CacheItem>` maintains insertion/access order.
- Every `GET` splices the accessed item to the **front** of the list.
- Every `SET` inserts at the **front**.
- When `store.size() >= capacity`, the item at the **back** (least recently used) is removed.

---

## 6. Pub/Sub Messaging

CacheX supports real-time publish/subscribe messaging between connected clients.

### Subscriber (Terminal 1)

```bash
./cachex
cachex> SUBSCRIBE alerts
SUBSCRIBED
```

The client now listens. When a message arrives on the `alerts` channel, it prints:

```
MESSAGE alerts server_rebooting
```

### Publisher (Terminal 2)

```bash
./cachex
cachex> PUBLISH alerts server_rebooting
PUBLISHED
```

---

## 7. Server Metrics

The `STATS` command returns atomic hit/miss/write counters:

```
cachex> STATS
Hits: 142
Misses: 7
Writes: 200
```

These counters are tracked using `std::atomic<int>` and are safe to read under concurrent load.

---

## 8. Master-Replica Replication

CacheX supports single-master, multi-replica replication. All writes to the master are forwarded to replicas in real time.

### Step 1 — Start the Master

```bash
./cachex --server --port 6379
```

### Step 2 — Start a Replica

```bash
./cachex --server --port 6380 --replicaof 127.0.0.1 6379
```

Output on the Replica:
```
[2024-06-27 10:01:00] [INFO] Server listening on port 6380
[2024-06-27 10:01:00] [INFO] Connecting to master 127.0.0.1:6379
[2024-06-27 10:01:00] [INFO] Connected to master, sending SYNC
```

### Step 3 — Write to the Master

```bash
./cachex --cluster 6379
cachex> SET order:500 confirmed
OK
```

### Step 4 — Read from the Replica

```bash
./cachex --cluster 6380
cachex> GET order:500
confirmed
```

**How it works internally:**
1. The replica connects to the master via TCP and sends a `SYNC` command.
2. The master takes a snapshot and registers the replica's socket.
3. Every subsequent `SET` or `DEL` on the master is forwarded to all registered replica sockets.
4. The replica parses incoming commands and applies them to its local cache.
5. If the connection drops, the replica retries every 5 seconds.

---

## 9. Cluster Sharding (Consistent Hashing)

CacheX distributes keys across multiple nodes using a **Consistent Hash Ring** with 100 virtual nodes per physical node.

### Step 1 — Start Multiple Servers

```bash
./cachex --server --port 6379
./cachex --server --port 6380
./cachex --server --port 6381
```

### Step 2 — Connect the Cluster Client

```bash
./cachex --cluster 6379,6380,6381
```

```
CacheX REPL Client Started. Ports: 6379 6380 6381
cachex> SET user:alice data_a
OK
cachex> SET user:bob data_b
OK
cachex> SET user:carol data_c
OK
```

Each key is hashed and routed to a deterministic node. The same key always maps to the same node, ensuring consistency.

**How it works internally:**
- `ConsistentHash` maintains a `std::map<size_t, std::string>` as the ring.
- Each physical node is mapped to 100 virtual positions via `std::hash(node + "#" + i)`.
- For a given key, `ring.lower_bound(hash(key))` finds the next clockwise node.
- Adding or removing a node only redistributes `~1/N` of keys (minimal disruption).

---

## 10. Persistence & Recovery

CacheX provides two persistence mechanisms that work together:

### Append-Only File (AOF)

Every write operation is logged to `aof.log` in real time:

```
SET user:1001 Alice
SET session:abc token_xyz
DEL user:1001
```

### Snapshot

When you press `Ctrl+C` to stop the server, CacheX catches the `SIGINT` signal and dumps the entire cache to `snapshot.bin` before exiting:

```
^C
[2024-06-27 10:30:00] [INFO] Caught signal. Shutting down gracefully...
[2024-06-27 10:30:00] [INFO] Graceful shutdown complete. Snapshot saved.
```

### Automatic Recovery

When the server restarts, it automatically:
1. Loads `snapshot.bin` (if present) to restore bulk state.
2. Replays `aof.log` (if present) to apply any writes that occurred after the last snapshot.

```
[2024-06-27 10:31:00] [INFO] Loading snapshot...
[2024-06-27 10:31:00] [INFO] Replaying AOF...
[2024-06-27 10:31:00] [INFO] Server listening on port 6379
```

---

## 11. Configuration

Edit `cache.conf` to customize server behavior:

```ini
# Port the server listens on
port=6379

# Maximum number of entries before LRU eviction kicks in
max_memory=1024

# Logging verbosity: INFO, WARN, ERROR
loglevel=INFO

# How often the AOF buffer is fsynced: everysec | always | no
aof_sync_strategy=everysec
```

Lines starting with `#` are treated as comments.

---

## 12. Docker Deployment

### Single Node

```bash
docker build -t cachex -f docker/Dockerfile .
docker run -p 6379:6379 cachex
```

### Using Docker Compose

```bash
cd docker
docker-compose up --build
```

### Run Benchmarks in Docker

```bash
docker run --rm cachex ./benchmark
```

### Run Tests in Docker

```bash
docker run --rm cachex ./test_cache
```

---

## 13. Running Tests & Benchmarks

### Unit Tests

The test suite uses the [Catch2](https://github.com/catchorg/Catch2) framework and covers:
- Basic `SET` / `GET` / `DEL` correctness
- LRU eviction behavior (inserting beyond capacity)
- Consistent hash ring node distribution

```bash
./test_cache
```

```
===============================================================================
All tests passed (7 assertions in 3 test cases)
```

### Performance Benchmark

Measures raw `SET` throughput on the cache engine (no networking overhead):

```bash
./benchmark
```

```
100K SETs took 2376ms
```

This translates to approximately **42,000 operations per second** with LRU tracking, TTL metadata, shared mutex locking, and AOF persistence all active.
