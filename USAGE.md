# How to Use CacheX

CacheX provides a distributed, in-memory caching system that you can run locally, shard across multiple instances, and manage via an interactive command-line interface.

## 1. Starting a Single Server

To start a standalone CacheX server node, run the executable with the `--server` flag:

```bash
./cachex --server
```
By default, this will start the server on port `6379`. It automatically loads settings from `cache.conf` if present.

To specify a custom port, use the `--port` flag:
```bash
./cachex --server --port 6380
```

## 2. Connecting the Interactive Client (REPL)

You can connect to your running server(s) using the built-in interactive shell. Run `cachex` without the `--server` flag. By default, it connects to port `6379`.

```bash
./cachex
```

Once inside the shell, you can execute standard cache commands:
```text
cachex> SET user:1 Alice EX 60
OK
cachex> GET user:1
Alice
cachex> STATS
Hits: 1
Misses: 0
Writes: 1
cachex> exit
```
*(The `EX 60` flag sets a TTL expiration of 60 seconds).*

## 3. Creating a Cluster (Client-Side Sharding)

CacheX supports scaling out by running multiple servers. You can instruct the interactive client to connect to an array of nodes. The client uses **Consistent Hashing** to deterministically route your keys to the correct physical server.

1. Open three terminal windows and start three servers on different ports:
```bash
./cachex --server --port 6379
./cachex --server --port 6380
./cachex --server --port 6381
```

2. Open a fourth terminal and start the client, passing the `--cluster` flag with a comma-separated list of ports:
```bash
./cachex --cluster 6379,6380,6381
```
Any `SET` or `GET` command executed in this shell will be seamlessly routed to the correct server in the ring.

## 4. Master-Replica Replication

You can set up replication for high availability. Write commands sent to the Master node are instantly synchronized to all Replica nodes.

1. Start the Master node:
```bash
./cachex --server --port 6379
```

2. Start a Replica node, telling it to replicate the Master:
```bash
./cachex --server --port 6380 --replicaof 127.0.0.1 6379
```
The Replica will connect to the Master, issue a `SYNC` command (triggering a snapshot dump on the master), and begin streaming live updates.

## 5. Pub/Sub Messaging

CacheX supports a Publisher/Subscriber model for real-time messaging.

1. **Subscriber Terminal:**
```bash
./cachex
cachex> SUBSCRIBE news_channel
SUBSCRIBED
```

2. **Publisher Terminal:**
```bash
./cachex
cachex> PUBLISH news_channel "Hello_World!"
PUBLISHED
```
The Subscriber terminal will immediately receive `MESSAGE news_channel "Hello_World!"`.

## 6. Graceful Shutdown & Recovery

CacheX ensures data safety via **Snapshots** and **Append-Only Files (AOF)**.
- If you shut down a server by pressing `Ctrl+C`, it gracefully intercepts the signal (`SIGINT`) and saves its entire memory state to `snapshot.bin`.
- All writes are concurrently appended to `aof.log`.
- When you restart the server, it instantly loads `snapshot.bin` and replays the commands in `aof.log` to completely recover your data.

## 7. Running the Test Suite

The project includes mathematical and behavioral unit tests written using the Catch2 framework.
```bash
./test_cache
```
This suite verifies that the LRU eviction policy drops the oldest keys, the TTL sweeping clears expired keys, and the Consistent Hashing ring distributes nodes mathematically correctly.
