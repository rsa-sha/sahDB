# sahDB --- Testing Plan

## General Testing

-   Basic SET/GET correctness tests for different key and value sizes.
-   Validate SAVE and restart recovery from savefile.
-   Verify TTL and EXPIRE logic for immediate and delayed expiration.
-   Ensure consistent behavior across restarts.
-   Validate protocol parsing for valid and invalid commands.

## Performance Testing

-   Measure throughput under increasing concurrent clients.
-   Compare basic GET/SET latency against Redis (single-node, no
    persistence).
-   Measure latency impact of SAVE under active write load.
-   Test performance with varying key distributions.

## Memory Testing

-   Monitor memory usage during long-running workloads.
-   Detect memory leaks by tracking RSS over time with repeated SET/DEL
    cycles.
-   Stress TTL heap usage with many expiring keys.
-   Evaluate allocator behavior under fragmentation-heavy workloads.

## Fool Testing

-   Random client disconnects during command execution.
-   Partial and malformed commands sent by clients.
-   Kill server during SAVE and restart; validate recovery.
-   Network delays, slow clients, and simulated packet loss.
-   Sudden termination during heavy concurrent operations.
