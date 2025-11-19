#ifndef HASH_H
#define HASH_H

#include "common.h"

typedef struct Entry {
    char *key;
    char *value;
    struct Entry *next;
    struct Entry *prev;
    time_t expiry;          // when the KV is to be purged
    size_t heap_index;      // for efficicent random removal
} Entry;

typedef struct HashTable {
    Entry **buckets;        // array of linked list heads
    size_t size;            // number of buckets
    size_t count;           // number of elements
} HashTable;

#define MOD 31


// Methods for users
void ht_init();
err_t hash_insert(char *, char *);
err_t hash_get(char *);
err_t hash_exists(char *);
err_t hash_delete(char *);
err_t hash_update_expiry(char *, time_t);
// err_t ht_set

#endif