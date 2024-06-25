# CacheX Architecture
- **TCP Server**: Listens on port 6379.
- **Cache Engine**: O(1) unordered_map backed with LRU Doubly Linked List.
- **Eviction**: Background thread sweeps TTLs.
- **Persistence**: Snapshot binary file + AOF log.
- **Distributed**: Uses Consistent Hashing for sharding nodes.

