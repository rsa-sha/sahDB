# sahDB --- Experiments

This directory contains **manual and semi-structured experiments**
performed on sahDB to explore correctness, concurrency, and failure
behavior.

These are **learning artifacts**, not polished benchmarks.

## Purpose

The goals of these experiments are to: - understand behavior under
concurrency and load - identify correctness bugs and edge cases -
evaluate durability and recovery assumptions - document real failure
scenarios

## Experiment Template

Each experiment should follow this structure:

### Experiment Name

Short, clear identifier.

### Goal

What behavior or invariant is being tested?

### Setup

Server config, client count, TTL settings, persistence mode.

### Steps

Commands or scripts used to run the experiment.

### Expected Behavior

What the system is supposed to do.

### Observed Behavior

What actually happened.

### Impact

Severity and scope of the issue (if any).

### Notes

Insights, surprises, or questions raised.

### Next Actions

Potential fixes, design changes, or further tests.

## Suggested Experiments

-   Random client disconnects under load
-   SAVE during heavy writes
-   TTL expiration accuracy under concurrency
-   Memory growth over long runtimes
-   Restart recovery after abrupt termination

This directory is meant to evolve alongside the system.
