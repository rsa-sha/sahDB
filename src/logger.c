#include "logger.h"
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Common methods
uint64_t get_size(int fd) {
    struct stat st;
    if (fstat(fd, &st)==0)
        return st.st_size;
    return 0;                   // Assumes empty file
}

uint64_t count_lines(int fd) {
    char buf[4096];
    ssize_t n = 0;
    uint64_t lines = 0;
    while ((n = read(fd, buf, sizeof(buf)))>0) {
        for (ssize_t i=0; i<n;i++) {
            if (buf[i] == '\n')lines++;
        }
    }
    return lines;
}

uint64_t now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL +
           (uint64_t)ts.tv_nsec / 1000000ULL;
}


// Method mappings

const log_ops user_ops = {
    .init   = user_init,
    .write  = user_write,
    .rotate = user_rotate,
    .close  = user_close
};

const log_ops aof_ops = {
    .init   = aof_init,
    .write  = aof_write,
    .rotate = aof_rotate,
    .close  = aof_close
};

const log_ops rdb_ops = {
    .init   = rdb_init,
    .write  = rdb_write,
    .rotate = rdb_rotate,
    .close  = rdb_close
};



void log_setup(log_f *log, log_t type, const char *file_name) {
    switch(type) {
        case F_USER:
            log->ops = &user_ops;
            break;
        case F_AOF:
            log->ops = &aof_ops;
            break;
        case F_RDB:
            log->ops = &rdb_ops;
            break;
        }
    log->fname = strdup(file_name);
    log->ops->init(log);
}


// USER side logs stuff
int user_init(log_f *log) {
    // open file in append mode
    log->fd = open(log->fname, O_WRONLY | O_CREAT | O_APPEND, 0644);
    log->buf_used = 0;

    log->size_b = get_size(log->fd);
    log->nlines = count_lines(log->fd);
    // sync mode stuff
    log->sync_mode = SYNC_M_YES;
    log->sync_interval_ms = 50;         // 50 milli-seconds
    log->last_sync_time = now_ms();
    log->sync_every_bytes = 1024;       // Sync after every 1KB written
    log->bytes_since_sync = 0;
    return 0;
}




int user_write(log_f *log, const void *data, size_t len) {
    return 0;
}
int user_rotate(log_f *log) {
    return 0;
}
int user_close(log_f *log) {
    return 0;
}




// AOF LOG functions
int aof_init(log_f *log) {
    return 0;
}
int aof_write(log_f *log, const void *data, size_t len) {
    return 0;
}
int aof_rotate(log_f *log) {
    return 0;
}
int aof_close(log_f *log) {
    return 0;
}




// RDB LOG functions
int rdb_init(log_f *log) {
    return 0;
}
int rdb_write(log_f *log, const void *data, size_t len) {
    return 0;
}
int rdb_rotate(log_f *log) {
    return 0;
}
int rdb_close(log_f *log) {
    return 0;
}

