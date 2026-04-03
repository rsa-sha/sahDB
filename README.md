# sahDB — A Research-Oriented Key-Value Store

sahDB is an experimental in-memory key-value store designed for **systems and database research**, with a focus on understanding performance trade-offs in modern KV storage engines.

---

## 🎯 Motivation

Modern key-value stores such as Redis are widely used in production systems, but they involve several trade-offs between:

- Memory usage
- Latency
- Throughput
- Persistence overhead

This project aims to **systematically study these trade-offs** by building a simplified KV store and evaluating different design strategies.

---

## 🔬 Research Goals

The primary goal of sahDB is not feature completeness, but **experimental clarity**.

We aim to explore:

- Impact of value size on performance
- Memory vs latency trade-offs
- Effects of TTL and expiration strategies
- (Upcoming) Compression strategies and their impact on:
  - Throughput
  - Latency (p50, p99)
  - Memory usage

---

## ⚙️ Current Features

- Basic KV operations:
  - `SET`, `GET`, `DELETE`, `EXISTS`
- TTL support:
  - `EXPIRE`, lazy expiration using heap
- Persistence:
  - RDB-like savefile system
  - Startup recovery from disk
- Config support
- CLI interface (Redis-like)

---

## 🧪 Benchmarking

We benchmark sahDB against Redis to understand fundamental performance differences.

### Metrics:
- Throughput (ops/sec)
- Latency (average, max, later p99)
- Memory usage
- Persistence overhead

### Workloads:
- Read-heavy (80/20)
- Write-heavy
- Mixed workloads
- Varying value sizes

---

## 🆚 Comparison with Redis

Redis serves as a **baseline reference system**, not a direct competitor.

We aim to answer:

- Under what conditions does a simpler KV store outperform Redis?
- How do design choices affect performance under different workloads?
- What trade-offs emerge when introducing compression or expiration strategies?

---

## 🚧 Upcoming Work

- Pluggable compression layer:
  - LZ4 / Zstd
- Adaptive compression policies
- Advanced workload generation:
  - Zipfian distribution
- Detailed latency analysis (p50, p99)
- Research paper-style evaluation

---

## 📊 Long-Term Goal

This project will culminate in a **research-style report / paper** analyzing:

> "Performance trade-offs in in-memory key-value stores under varying workload and storage strategies"

---

## 🛠️ Getting Started

```bash
make
./build/sahDB
