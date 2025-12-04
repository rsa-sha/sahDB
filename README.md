# sahDB
Minimal Database in C

---
### 📘 Feature Overview & Roadmap
| Feature                        | Parts  completed                                                                             | Completion |
| ------------------------------ | -------------------------------------------------------------------------------------------- | ---------- |
| **Config Support**             | Load settings from config file; added ability to fetch key expiry UNIX time via `get key ex` | [ ]        |
| **Savefile System (RDB-like)** | Savefile version 001; ability to rebuild DB from savefile on startup                         | [ ]        |
| **Lazy Expiration + Heap**     | Lazy TTL expiration check using heap                                                         | [ ]        |
| **TTL & EXPIRE Command**       | Implemented `EXPIRE` command and TTL logic; formatting updates                               | [ ]        |
| **Network Layer (Go)**         | Fix for issue #9; started network implementation (#10, #11)                                  | [x]        |
| **SAVE Command**               | `SAVE` command implementation & tests for same                                               | [x]        |
| **Hash Table Implementation**  | Migrated to basic hash table; hashing changes; merged initial PR                             | [x]        |

### 📌 Planned / Upcoming Features

| Feature                              | About                                                                                            | Completion |
| ------------------------------------ | ------------------------------------------------------------------------------------------------ | ---------- |
| **Concurrency Model**                | Define single-threaded or sharded-thread model; ensure atomic commands; test simultaneous writes | [ ]        |
| **Write-Ahead Log (WAL / AOF)**      | Add append-only log for durability; replay on startup; log rotation                              | [ ]        |
| **Replication (Leader → Follower)**  | Full sync + live command streaming; design sync protocol                                         | [ ]        |
| **Binary-Safe Client Protocol**      | Design RESP-like or custom length-prefixed protocol                                              | [ ]        |
| **Active Expiration**                | Background TTL purging in addition to lazy-expire                                                | [ ]        |
| **Memory Management Improvements**   | Custom allocators, object pooling, fragmentation control                                         | [ ]        |
| **Partial Resync & Backlog**         | Incremental replication using offsets + backlog                                                  | [ ]        |
| **Sharding / Clustering (Optional)** | Hash-based partitioning; scaling across nodes                                                    | [ ]        |
