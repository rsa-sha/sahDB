#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdint.h>
#include <time.h>

/*
 * Log / Persistence Write Path Overview
 * -------------------------------------
 *
 * This module implements a buffered, append-only write system with
 * optional durability (fsync) and log rotation. It is designed to be
 * reusable across:
 *   - user logs (low durability)
 *   - AOF (append-only, durability-sensitive)
 *   - RDB (snapshot-style writes)
 *
 *
 * CORE PIPELINE
 * -------------
 *   [APP BUFFER] → write() → [KERNEL PAGE CACHE] → fsync() → [DISK]
 *
 *   - Buffer: batches small writes (performance)
 *   - write(): transfers data to OS (NOT durable)
 *   - fsync(): guarantees data is persisted to disk
 *
 *
 * WRITE FLOW (append operation)
 * -----------------------------
 *
 * 1. Buffer Check
 *    If incoming data does not fit in buffer:
 *        → flush_buffer()  (write to fd)
 *
 * 2. Append to Buffer
 *        memcpy(buffer + buf_used, data, len)
 *        buf_used += len
 *
 * 3. Update Logical State (IMPORTANT)
 *        size_b  += len
 *        nlines  += number_of_newlines(data)
 *
 *    NOTE: This reflects the "intended" file state, even if buffer
 *    has not yet been flushed to the OS.
 *
 *
 * 4. Rotation Check (AFTER state update)
 *    If any limit exceeded (size, lines, age):
 *        → flush_buffer()
 *        → fsync(fd)        // recommended for safety
 *        → close(fd)
 *        → rename(old → rotated)
 *        → open(new file with O_APPEND)
 *
 *    NOTE: Rotation is independent of buffer capacity.
 *
 *
 * 5. Buffer Flush (when triggered)
 *        write(fd, buffer, buf_used)
 *        bytes_since_fsync += buf_used
 *        buf_used = 0
 *
 *
 * FSYNC (DURABILITY POLICY)
 * -------------------------
 *
 * fsync() is controlled by policy and is NOT tied to buffer usage.
 *
 * Supported strategies:
 *
 *   1. ALWAYS
 *        fsync after every write/flush
 *
 *   2. INTERVAL (sync_interval_ms > 0)
 *        if (now - last_fsync >= interval)
 *            fsync(fd)
 *
 *   3. BYTES (sync_every_bytes > 0)
 *        if (bytes_since_fsync >= threshold)
 *            fsync(fd)
 *
 *   4. NONE (interval == 0 && bytes == 0)
 *        no automatic fsync (OS decides flush timing)
 *
 * On fsync:
 *        last_fsync = now
 *        bytes_since_fsync = 0
 *
 *
 * IMPORTANT DISTINCTIONS
 * ----------------------
 *
 * - Buffer flush != durability
 *      write() only moves data to kernel page cache
 *
 * - fsync() defines the crash-safe boundary
 *
 * - Kernel may delay writes indefinitely without fsync()
 *
 * - There is no way to query when page cache is written to disk;
 *   we track durability manually via last_fsync timestamp.
 *
 *
 * ROTATION SAFETY
 * ---------------
 *
 * Before rotating:
 *        flush_buffer()
 *        fsync(fd)   // ensures old file is fully persisted
 *
 * Failing to fsync before rotation may result in data loss
 * in the rotated file after a crash.
 *
 *
 * DESIGN PRINCIPLES
 * -----------------
 *
 * - Buffering is for performance (batching writes)
 * - write() hands data to OS (not durable)
 * - fsync() enforces durability (policy-driven)
 * - Rotation is based on logical file state, not buffer state
 * - Policy (when to fsync) must remain separate from I/O mechanism
 *
 *
 * Log level controls what gets logged
 * INFO → user-safe
 * DEBUG → developer-focused
 * TRACE → deep internals

 * Level	Default flags
   - INFO	timestamp, level, category
   - DEBUG	+ function
   - TRACE	+ file + line
 */
#define LL_INFO     0
#define LL_WARN     1
#define LL_ERR      2
#define LL_DEBUG    3
#define LL_TRACE    4

// define LOG types
typedef enum log_t{
    F_USER,
    F_AOF,
    F_RDB
} log_t;


#define SYNC_M_NO       (1<<0)
#define SYNC_M_YES      (1<<1)
#define SYNC_M_OTH      (1<<2)

// Forward declaration of log_f struct
typedef struct log_f log_f;

// Methods for logging

/* Init */
typedef int (*log_init_fn)(log_f *log);

/* Common ops */
typedef int (*log_write_fn)(log_f *log, const void *data, size_t len);
typedef int (*log_rotate_fn)(log_f *log);
typedef int (*log_close_fn)(log_f *log);

// Log operations sub-struct
typedef struct log_ops {
    log_init_fn    init;
    log_write_fn   write;
    log_rotate_fn  rotate;
    log_close_fn   close;
} log_ops;


// BASE log file struct, to be made as reusable entity for AOF, RDB
typedef struct log_f {
    int         fd;                 // FD to the log file
    char        *fname;             // Name of the log file

    uint64_t    size_b;             // Current size of log in bytes
    uint64_t    nlines;             // Will help during rotation
    time_t      creat;              // Creation time of file, useful on rotation

    void        *f_rotat;           // Pointer to the rotator function
    char        buffer[4*1024];     // 4 KB buffer, which gets flushed
    size_t      buf_used;           // Amount of buffer already used, guides on when to flush
    
    // FSYNC things
    uint8_t     sync_mode;          // Bitwise sync mode flag var
    uint32_t    sync_interval_ms;   // After how many ms fsync should be triggered
    uint64_t    last_sync_time;     // Stores time in ms of last fsync
    uint32_t    sync_every_bytes;   // After how many bytes written fsync to be triggered
    uint32_t    bytes_since_sync;   // How many bytes written to using `write`, used to track for fsync

    // Operations struct
    const log_ops *ops;
} log_f;


// USER LOG functions
int user_init(log_f *log);
int user_write(log_f *log, const void *data, size_t len);
int user_rotate(log_f *log);
int user_close(log_f *log);




// AOF LOG functions
int aof_init(log_f *log);
int aof_write(log_f *log, const void *data, size_t len);
int aof_rotate(log_f *log);
int aof_close(log_f *log);



// RDB LOG functions
int rdb_init(log_f *log);
int rdb_write(log_f *log, const void *data, size_t len);
int rdb_rotate(log_f *log);
int rdb_close(log_f *log);


// Called during server start-up
void log_setup(log_f *log, log_t type, const char *file_name);

/*
 * HOW the LOG could show up in the file:
   - [timestamp] [level] [category] [function] message
   - 01011970:000001 [DEBUG][STORAGE][flush_page] Flushing page 42
   - 01011970:000001 [TRACE][STORAGE][flush_page][storage.c:214] Writing page 42
 */
#endif
