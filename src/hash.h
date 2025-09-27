#ifndef HASH_H
#define HASH_H

#include "common.h"

typedef struct Entry{
    char *key;
    char *value;
    struct Entry *next;     // multiple entries with same Hash value
} Entry;

typedef struct HashTable {
    Entry **buckets;        // array of linked list heads
    size_t size;            // number of buckets
    size_t count;           // number of elements
} HashTable;

#define MOD 31


// Methods for users
void ht_init();
int hash_insert(char*, char*);
int hash_get(char*);
int hash_exists(char*);
int hash_delete(char*);
// err_t ht_set

#endif