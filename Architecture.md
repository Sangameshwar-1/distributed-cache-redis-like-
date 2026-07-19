# CacheX Architecture

CacheX is a custom, high-performance, distributed in-memory caching engine written entirely in C++. It was designed from the ground up for the IIIT Felicity showcase to serve as a drop-in, optimized replacement for Redis in the MERN-STACK backend.

## System Overview

```mermaid
graph TD
    subgraph Frontend
        React[React Client]
    end

    subgraph "MERN Backend (Node.js)"
        Node[Express API]
        Client[CacheX TCP Client]
        Node <--> Client
    end

    subgraph "CacheX Engine (C++)"
        Server[TCP Server]
        Pool[Thread Pool]
        Core[In-Memory Store]
        Sync[Replication Sync]
        
        Server --> Pool
        Pool --> Core
        Core --> Sync
    end

    subgraph Persistence
        AOF[Append-Only File]
        Snap[Binary Snapshot]
    end
    
    subgraph Database
        Mongo[(MongoDB)]
    end

    React <-->|HTTP REST| Node
    Client <-->|TCP Socket (Base64 JSON)| Server
    Node <-->|Mongoose| Mongo
    Core -->|Sync| AOF
    Core -->|Backup| Snap
```

## Key Components

### 1. The Custom TCP Protocol
Instead of relying on the complex RESP (REdis Serialization Protocol), CacheX uses a lightweight, newline-delimited text protocol.
- **Request Format:** `COMMAND KEY VALUE\n`
- **Response Format:** `OK\n` or `VALUE\n` or `(nil)\n`

To safely transport complex JSON objects (like the MERN stack event lists) over this raw text stream, the Node.js client automatically Base64 encodes payloads before sending them, and decodes them upon receipt.

### 2. Thread-Safe In-Memory Core
The core data structure is an `std::unordered_map` paired with an `std::list`.
- **Locking:** We use `std::shared_mutex`. This allows **multiple simultaneous readers** (cache hits) without blocking, but requests an exclusive lock when a write occurs. This guarantees data consistency without sacrificing read throughput.
- **LRU Eviction:** The `std::list` tracks the Least Recently Used keys. When the cache hits its capacity (default 1000 items), the oldest item at the back of the list is evicted.

### 3. TTL & Background Sweeping
Items can optionally be stored with a Time-To-Live (TTL). A dedicated background thread wakes up every second to sweep the memory map and proactively delete expired keys, ensuring memory is never wasted on stale data.

### 4. Persistence
CacheX provides durable storage so data isn't lost on server restarts:
1. **AOF (Append-Only File)**: Every write operation (`SET`, `DEL`) is immediately logged to a file.
2. **Snapshots**: Periodic binary snapshots of the entire memory state are saved to disk. On startup, the engine loads the snapshot and replays the AOF to reach the exact previous state.

### 5. Replication & Consistent Hashing
For scaling beyond a single node:
- **Master/Replica Sync**: Replicas can connect to a master node. The master streams its AOF to all replicas in real-time.
- **Consistent Hashing**: A hashing ring with virtual nodes distributes keys evenly across a cluster, ensuring minimal cache misses if a node goes down or is added.
