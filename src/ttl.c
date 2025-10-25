#include "hash.h"
#include "ttl.h"

ttl_heap *ttl = NULL;

bool compare_ttl(ttl_node *a, ttl_node *b) {
    if (a->e->expiry <= b->e->expiry)
        return true;
    return false;
}


void heapify(ttl_heap *h, size_t i) {
    size_t smallest = i;
    size_t l = LEFT(i);
    size_t r = RIGHT(i);

    if (l < h->size && compare_ttl(&h->heap[l], &h->heap[smallest]))
        smallest = l;

    if (r < h->size && compare_ttl(&h->heap[r], &h->heap[smallest]))
        smallest = r;

    if (smallest != i) {
        ttl_node temp = h->heap[i];
        h->heap[i] = h->heap[smallest];
        h->heap[smallest] = temp;
        heapify(h, smallest);
    }
}


err_t heap_insert(ttl_heap *h, Entry *e) {
    if (h->size == h->capacity) {
        size_t new_cap = h->capacity? h->capacity*2:16;
        ttl_node *new_heap = realloc(h->heap, new_cap * sizeof(ttl_node));
        if (!new_heap) return DB_ERR_NOMEM;
        h->heap = new_heap;
        h->capacity = new_cap;
    }

    size_t i = h->size++;
    h->heap[i].e = e;

    // move up
    while (i>0 && compare_ttl(&h->heap[i], &h->heap[PARENT(i)])) {
        ttl_node tmp = h->heap[PARENT(i)];
        h->heap[PARENT(i)] = h->heap[i];
        h->heap[i] = tmp;
        i = PARENT(i);
    }
    return DB_ERR_OK;
}

Entry *heap_pop(ttl_heap *h) {
    if (h->size == 0)
        return NULL;
    Entry *min = h->heap[0].e;
    h->heap[0] = h->heap[h->size-1];
    h->size--;

    heapify(h, 0);
    return min;
}

void ttl_init() {
    ttl = (ttl_heap *)malloc(sizeof(ttl_heap));
    if (!ttl) {
        perror("Mem alloation failed");
        return;
    }
    ttl->size = 0;
    ttl->capacity = 16;
    ttl->heap = (ttl_node *)malloc(sizeof(ttl_node)*ttl->capacity);
    if (!ttl->heap) {
        perror("Mem allcation failed");
        free(ttl);
        return;
    }
}