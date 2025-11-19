#ifndef TTL_H
#define TTL_H

#include "common.h"
#include "hash.h"

// Forward declaration of Entry to ensure it's recognized
typedef struct Entry Entry;

// STRUCT (min-heap) for storing pointers of eligible KVs

typedef struct ttl_node {
    Entry *e;
} ttl_node;


typedef struct ttl_heap {
    ttl_node *heap;
    size_t size;
    size_t capacity;
} ttl_heap;


#define PARENT(i) ((i-1)/2)
#define LEFT(i) ((i*2) + 1)
#define RIGHT(i) ((i*2) + 2)

extern ttl_heap *ttl;

void ttl_init();
void ttl_cleanup();
/*
Compares the expiry time of entries A and B
if A is to expire before or at the same time as B
    returns True
else
    returns False
*/
bool compare_ttl(ttl_node *a, ttl_node *b);

void heapify(ttl_heap *h, size_t i);

// To add element in the ttl heap [used by hash_update_expiry]
err_t heap_insert(ttl_heap *h, Entry *e);

// For removing element before expiry
err_t heap_remove(ttl_heap *h, Entry *e);

/*
Removes the entry of the expired KV from the TTL heap
Returns the same entry, which is then removed from the HashTable
*/
Entry* heap_pop(ttl_heap *h);

#endif