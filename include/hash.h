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
// this method returns the KV entry and is for INTERNAL USE ONLY
Entry* hash_get_kv(char *);

/*
Info: Fetches expiry of KV from HT
Input: Key (char*)
Resp: UNIX expiry time if expiry is set
Return: (0=>Expiry Exists, 4=>Key does not exist)
*/
err_t hash_get_expiry(char *);

/*
Info: Verifies existence of KV in HT
Input: Key (char*)
Resp: TRUE||FALSE based on existence
Return: (0=>Exists, 4=>Key does not exist)
*/
err_t hash_exists(char *);

/*
Info: Removes entry of KV from HT
Input: Key (char*)
Resp: Confirmation of deletion|Log in case non-existent Key
Return: (0=>YES, 4=>Key does not exist)
*/
err_t hash_delete(char *);
err_t hash_update_expiry(char *, time_t);
// err_t ht_set

#endif