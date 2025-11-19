#include "hash.h"
#include "ttl.h"

ttl_heap *ttl = NULL;

static inline void swap_nodes(ttl_heap *h, size_t a, size_t b) {
    ttl_node tmp = h->heap[a];
    h->heap[a] = h->heap[b];
    h->heap[b] = tmp;

    h->heap[a].e->heap_index = a;
    h->heap[b].e->heap_index = b;
}

bool compare_ttl(ttl_node *a, ttl_node *b) {
    if (a->e->expiry <= b->e->expiry)
        return true;
    return false;
}


static void shift_up(ttl_heap *h, size_t i) {
    while (i > 0 && compare_ttl(&h->heap[i], &h->heap[PARENT(i)])) {
        swap_nodes(h, i, PARENT(i));
        i = PARENT(i);
    }
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
        swap_nodes(h, i, smallest);
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
    e->heap_index = i;

    // move up
    shift_up(h, i);
    return DB_ERR_OK;
}

Entry *heap_pop(ttl_heap *h) {
    if (h->size == 0)
        return NULL;
    Entry *min = h->heap[0].e;
    swap_nodes(h, 0, h->size - 1);
    h->size--;

    if (h->size > 0)
        heapify(h, 0);
    
    min->heap_index = SIZE_MAX;
    return min;
}

err_t heap_remove(ttl_heap *h, Entry *e) {
    size_t i = e->heap_index;
    if (i>=h->size || h->heap[i].e != e)
        return DB_ERR_KEY_NOTEXIST;

    // If removing the last element, just decrement size
    if (i == h->size - 1) {
        h->size--;
        e->heap_index = SIZE_MAX;
        return DB_ERR_OK;
    }

    // Swap with last element
    swap_nodes(h, i, h->size - 1);
    h->size--;
    e->heap_index = SIZE_MAX;

    // Fix heap property at index i
    // Check if we need to sift up or down
    if (i > 0 && compare_ttl(&h->heap[i], &h->heap[PARENT(i)])) {
        shift_up(h, i);
    } else {
        heapify(h, i);
    }
    
    return DB_ERR_OK;
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