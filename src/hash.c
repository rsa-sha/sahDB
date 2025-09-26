#include "common.h"
#include "hash.h"

HashTable* ht;

static uint64_t string_folding_hash(char* k){
    uint64_t sm = 0;
    uint64_t mul = 1;
    for(uint64_t i=0;i<strlen(k);i++){
        mul = (i%4==0)?1:256;
        sm = (sm + k[i]*mul)%MOD;
    }
    return (int)sm%MOD;
}

int hash_insert(char* k, char* v){
    int idx = string_folding_hash(k);
    Entry* kv = malloc(sizeof(Entry));
    if(!kv)return -1;
    kv->key = strdup(k);
    kv->value = strdup(v);
    // add_kv_in_arr()
    return 0;
}

void ht_init(){
    ht = malloc(sizeof(HashTable));
    ht->size = MOD;
    ht->count = 0;
    ht->buckets = calloc(MOD, sizeof(Entry));
}