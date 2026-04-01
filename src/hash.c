#include "common.h"
#include "hash.h"
#include "ttl.h"

HashTable* ht;

static uint64_t string_folding_hash(char *k) {
    // DJB2 algorithm
    uint64_t hash = 5381;
    int c;
    while ((c = *k++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash % MOD;
}

static err_t add_kv_in_arr(int idx, Entry *e) {
    if (ht->count + 1 >= ht->size) {
        // We still need to check if it's an update even if full
        Entry* temp = ht->buckets[idx];
        while (temp!=NULL) {
            if (strcmp(temp->key, e->key) == 0) {
                free(temp->value);
                temp->value = strdup(e->value);
                return DB_ERR_KEY_EXISTS;
            }
            temp = temp->next;
        }
        return ERR_FULL;
    }
    Entry* temp = ht->buckets[idx];
    if (temp==NULL) {
        ht->buckets[idx] = e;
        ht->count++;
        return 0;
    }
    while (temp->next!=NULL && strcmp(temp->key, e->key)!=0)
        temp = temp->next;
    // check if a KV is being update
    if (strcmp(temp->key, e->key)==0) {
        free(temp->value);
        temp->value = strdup(e->value);
        return DB_ERR_KEY_EXISTS;
    }
    // adding value at end of list
    temp->next = e;
    ht->count++;
    return 0;
}

static err_t lazy_expire_and_delete(Entry *e) {
    err_t res = DB_ERR_KEY_NOT_EXPIRED;
    if (e->expiry > time(NULL) || e->expiry==-1)
        return res;
    res = DB_ERR_KEY_EXPIRED;
    // deleting the key
    heap_remove(ttl, e);
    SILENT = true;
    hash_delete(e->key, NULL);
    SILENT = false;
    return res;
}


err_t hash_get_expiry(char *k, cmd_ctx *ctx) {
    char resp[MAX_RESP_LEN];
    err_t res = 0;
    int idx = string_folding_hash(k);
    Entry* temp = ht->buckets[idx];
    while (temp!=NULL && strcmp(temp->key, k)!=0)
        temp = temp->next;
    if (temp==NULL || strcmp(temp->key,k)!=0 || lazy_expire_and_delete(temp)==DB_ERR_KEY_EXPIRED) {
        sprintf(resp, "%sValue corresponding to key %s not present in DB%s", RED, k, RESET);
        res = DB_ERR_KEY_NOTEXIST;
        goto ret;
    }
    if (strcmp(temp->key, k)==0) {
        if (temp->expiry!=-1)
            sprintf(resp, "Expiry at UNIX time %zu",temp->expiry);
        else
            sprintf(resp, "%s%sNo expiry set for %s%s", BOLD, RED, k, RESET);
    }
ret:
    fill_cmd_ctx(ctx, res, resp);
    return res;
}

err_t hash_get(char *k, cmd_ctx *ctx) {
    char resp[MAX_RESP_LEN];
    err_t res = 0;
    int idx = string_folding_hash(k);
    Entry* temp = ht->buckets[idx];
    while (temp!=NULL && strcmp(temp->key, k)!=0)
        temp = temp->next;
    if (temp==NULL || strcmp(temp->key,k)!=0 || lazy_expire_and_delete(temp)==DB_ERR_KEY_EXPIRED) {
        sprintf(resp, "%sValue corresponding to key %s not present in DB%s", RED, k, RESET);
        res = DB_ERR_KEY_NOTEXIST;
        goto ret;
    }
    if (strcmp(temp->key, k)==0)
        sprintf(resp, "%s",temp->value);
ret:
    fill_cmd_ctx(ctx, res, resp);
    return res;
}

err_t hash_exists(char *k, cmd_ctx *ctx) {
    char resp[MAX_RESP_LEN];
    err_t res = 0;
    int idx = string_folding_hash(k);
    Entry* temp = ht->buckets[idx];
    while (temp!=NULL && strcmp(temp->key, k)!=0)
        temp = temp->next;
    if (temp==NULL || strcmp(temp->key,k)!=0 || lazy_expire_and_delete(temp)==DB_ERR_KEY_EXPIRED) {
        sprintf(resp, "%s%sFALSE%s", BOLD, RED, RESET);
        res = DB_ERR_KEY_NOTEXIST;
        goto ret;
    }
    if (strcmp(temp->key, k)==0)
        sprintf(resp, "%s%sTRUE%s", BOLD, GREEN, RESET);
ret:
    fill_cmd_ctx(ctx, res, resp);
    return res;
}

err_t hash_delete(char *k, cmd_ctx *ctx) {
    int idx = string_folding_hash(k);
    char resp[MAX_RESP_LEN];
    err_t res = 0;
    Entry *cur = ht->buckets[idx];
    Entry *prev = NULL;
    while ((cur!=NULL) && strcmp(cur->key, k)!=0) {
        prev = cur;
        cur = cur->next;
    }
    // check if the key exists or not
    if (cur==NULL) {
        sprintf(resp, "%sNo entry corresponding to key %s in DB%s", RED, k, RESET);
        res = DB_ERR_KEY_NOTEXIST;
        goto ret;
    }
    // else there is
    if (prev==NULL) {
        // first entry is to be removed
        ht->buckets[idx] = cur->next;
    } else {
        prev->next = cur->next;
    }
    ht->count--;
    // removing from TTL in case it's present
    if (cur->heap_index!=SIZE_MAX && cur->expiry!=LONG_MAX)
        heap_remove(ttl, cur);
    free(cur->key);
    free(cur->value);
    free(cur);
    sprintf(resp, "%sEntry corresponding to key %s removed from DB%s", GREEN, k, RESET);
ret:
    fill_cmd_ctx(ctx, res, resp);
    return res;
}

err_t hash_insert(char *k, char *v, cmd_ctx *ctx) {
    int idx = string_folding_hash(k);
    Entry *kv = malloc(sizeof(Entry));
    if (!kv) return DB_ERR_NOMEM;
    kv->key = strdup(k);
    kv->value = strdup(v);
    kv->next = NULL;
    kv->prev = NULL;
    kv->heap_index = SIZE_MAX;
    kv->expiry = -1;

    err_t res = add_kv_in_arr(idx, kv);
    char resp[MAX_RESP_LEN];
    if (res == 0) {
        sprintf(resp, "Added key %s and value %s in DB", kv->key, kv->value);
    } else if (res == DB_ERR_KEY_EXISTS) {
        sprintf(resp, "Updated key %s with value %s in DB", k, v);
        free(kv->key);
        free(kv->value);
        free(kv);
        res = 0; // Return success for update
    } else if (res == ERR_FULL) {
        sprintf(resp, "DB storage is full");
        sprintf(resp, "DB storage is full");
        free(kv->key);
        free(kv->value);
        free(kv);
    }
    fill_cmd_ctx(ctx, res, resp);
    return res;
}

Entry* hash_get_kv(char *key) {
    int idx = string_folding_hash(key);
    Entry *ret = NULL;
    Entry *kv = ht->buckets[idx];
    while (kv!=NULL && (strcmp(kv->key, key)!=0))
        kv = kv->next;
    if (kv==NULL || lazy_expire_and_delete(kv)==DB_ERR_KEY_EXPIRED) {
        goto ret;
    } else {        
        ret = kv;
    }
ret:
    return ret;
}

err_t hash_update_expiry(char *key, time_t duration, cmd_ctx *ctx) {
    int idx = string_folding_hash(key);
    Entry *kv = ht->buckets[idx];
    err_t res = DB_ERR_OK;
    char resp[100];
    while (kv!=NULL && (strcmp(kv->key, key)!=0))
        kv = kv->next;
    if (kv==NULL || lazy_expire_and_delete(kv)==DB_ERR_KEY_EXPIRED) {
        res = DB_ERR_KEY_NOTEXIST;
        sprintf(resp, "%sNo entry for key %s in DB%s", RED, key, RESET);
    } else {
        // If already in heap, remove it first
        if (kv->heap_index != SIZE_MAX) {
            heap_remove(ttl, kv);
        }
        
        kv->expiry = time(NULL) + duration;
        
        // adding KV to ttl
        res = heap_insert(ttl, kv);
        if (res != DB_ERR_OK) {
            sprintf(resp, "%sUnable to set expiry for key %s%s", RED, key, RESET);
            goto ret;
        }
        sprintf(resp, "%sUpdated expiry time of key %s%s", GREEN, key, RESET);
    }
ret:
    fill_cmd_ctx(ctx, res, resp);
    return res;
}


void ht_init() {
    ht = malloc(sizeof(HashTable));
    if (ht == NULL) {
        perror("Mem allocation failed");
        return;
    }
    ht->size = MOD;
    ht->count = 0;
    ht->buckets = calloc(MOD, sizeof(Entry *));
}
