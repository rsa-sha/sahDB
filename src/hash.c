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

static err_t add_kv_in_arr(int idx, Entry *e){
    if(ht->count + 1 >= ht->size)
        return ERR_FULL;
    Entry* temp = ht->buckets[idx];
    if(temp==NULL){
        ht->buckets[idx] = e;
        ht->count++;
        return 0;
    }
    while(temp->next!=NULL && strcmp(temp->key, e->key)!=0){
        temp = temp->next;
    }
    // check if a KV is being update
    if(strcmp(temp->key, e->key)==0){
        free(temp->value);
        // temp->value = 
        strcpy(temp->value, e->value);
        return 0;
    }
    // adding value at end of list
    temp->next = e;
    ht->count++;
    return 0;
}

int hash_get(char* k){
    char resp[MAX_RESP_LEN];
    err_t res = 0;
    int idx = string_folding_hash(k);
    Entry* temp = ht->buckets[idx];
    while(temp!=NULL && strcmp(temp->key, k)!=0){
        temp = temp->next;
    }
    if(temp==NULL || strcmp(temp->key,k)!=0){
        sprintf(resp, "%sValue corresponding to key %s not present in DB%s", RED, k, RESET);
        res = 1;
        goto ret;
    }
    if(strcmp(temp->key, k)==0)
        sprintf(resp, "%s",temp->value);
ret:
    send_info_to_user(resp);
    return res;
}

int hash_exists(char* k){
    char resp[MAX_RESP_LEN];
    err_t res = 0;
    int idx = string_folding_hash(k);
    Entry* temp = ht->buckets[idx];
    while(temp!=NULL && strcmp(temp->key, k)!=0){
        temp = temp->next;
    }
    if(temp==NULL || strcmp(temp->key,k)!=0){
        sprintf(resp, "%s%sFALSE%s", BOLD, RED, RESET);
        res = 1;
        goto ret;
    }
    if(strcmp(temp->key, k)==0)
        sprintf(resp, "%s%sTRUE%s", BOLD, GREEN, RESET);
ret:
    send_info_to_user(resp);
    return res;
}

int hash_delete(char* k){
    int idx = string_folding_hash(k);
    char resp[MAX_RESP_LEN];
    err_t res = 0;
    Entry* cur = ht->buckets[idx];
    Entry* prev = NULL;
    while((cur!=NULL) && strcmp(cur->key, k)!=0){
        prev = cur;
        cur = cur->next;
    }
    // check if the key exists or not
    if(cur==NULL){
        sprintf(resp, "%sNo entry corresponding to key %s in DB", RED, RESET);
        res = -1;
        goto ret;
    }
    // else there is
    if(prev==NULL){
        // first entry is to be removed
        ht->buckets[idx] = cur->next;
    } else{
        prev->next = cur->next;
    }
    ht->count--;
    free(cur->key);
    free(cur->value);
    free(cur);
    sprintf(resp, "%sEntry corresponding to key %s removed from DB%s", GREEN, k, RESET);
ret:
    send_info_to_user(resp);
    return res;
}

int hash_insert(char* k, char* v){
    int idx = string_folding_hash(k);
    Entry* kv = malloc(sizeof(Entry));
    if(!kv)return -1;
    kv->key = strdup(k);
    kv->value = strdup(v);
    kv->next = NULL;
    err_t res = add_kv_in_arr(idx, kv);
    char resp[MAX_RESP_LEN];
    if (res==0)
        sprintf(resp, "Added key %s and value %s in DB",
            kv->key, kv->value);
    else if(res==ERR_FULL)
        sprintf(resp, "DB storage is full");
    send_info_to_user(resp);
    // free(kv->value);
    // free(kv->key);
    // free(kv);
    return res;
}


void ht_init(){
    ht = malloc(sizeof(HashTable));
    if (ht == NULL) {
        perror("Mem allocation failed");
        return;
    }
    ht->size = MOD;
    ht->count = 0;
    ht->buckets = calloc(MOD, sizeof(Entry));
}