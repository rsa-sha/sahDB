#include "dbsave.h"

int process_save = 0;

static void process_save_data(char *line) {
    // char *data = strtok(line, "\n");
    if (strcmp(line, "$") == 0) {
        process_save ^= 1;
    } else {
        if (process_save) {
            // if multiple entries
            char *saveptr_1, *saveptr_2;
            char *kv = strtok_r(line, "|", &saveptr_1);
            while(kv!=NULL) {
                char *key = strtok_r(kv, "!", &saveptr_2);
                char *val = strtok_r(NULL, "!", &saveptr_2);
                char *exp = strtok_r(NULL, "!", &saveptr_2);
                hash_insert(key, val);
                if(strcmp(exp, "-1")) {
                    time_t expiry_time = (time_t)atol(exp);
                    time_t curtime = time(NULL);
                    if (expiry_time <= curtime) {
                        hash_delete(key);
                        // TODO: To internal server log
                        // send_info_to_user("The key %s is already expired", key);
                    } else {
                        Entry *e = hash_get_kv(key);
                        if (e)
                            e->expiry = expiry_time;
                        free(e);
                    }
                }
                kv = strtok_r(NULL, "|", &saveptr_1);
            }
        }
    }
}

err_t rebuild_from_savefile() {
    // verify file version
    // char fver[25];
    err_t res = DB_ERR_OK;
    char resp[MAX_RESP_LEN];
    FILE *fp = fopen(SAVE_FILE, "r");
    if (fp == NULL) {
        res = DB_ERR_GENERIC_FAIL;
        goto ret;
    }
    char line[MAX_RESP_LEN];
    while(fgets(line, sizeof(line), fp) != NULL) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';  // Replace the newline with null terminator
        }
        SILENT = 1;
        process_save_data(line);
        SILENT = 0;
        // *line = NULL;
    }
    sprintf(resp, "FILE READ!");
ret:
    // TODO: Need to implement logging function
    send_info_to_user(resp);
    return res;
}


err_t fetch_dataset_from_memory() {
    err_t res = 0;
    FILE *fp = fopen(SAVE_FILE, "w");
    char resp[MAX_RESP_LEN] = {0};
    if (!fp) {
        sprintf(resp, "Error opening %s file", SAVE_FILE);
        res = DB_ERR_FILE_INACCESSIBLE;
        goto ret;
    }
    // Write metadata
    // vers000
    // fprintf(fp, "{\n");
    // fprintf(fp, "   \"count\": %zu,\n", ht->count);
    // fprintf(fp, "   \"size\": %zu,\n", ht->size);
    // fprintf(fp, "   \"data\": [\n");
    // int n_buckets = 0;
    // char dataset[ht->count*KV_LEN];
    // VERS 001
    // write header
    fprintf(fp, "%s\n", FILE_HEADER_VER);
    for (int i=0;i<ht->size;i++) {
        char* bucket_data = get_kv_from_bucket(ht->buckets[i]);
        if (bucket_data && strlen(bucket_data)>0)
            // fprintf(fp, "       {\"bucket\":%d, \"entries\":%s},\n", i, bucket_data);
            fprintf(fp, "$\n%s\n$\n", bucket_data);
        free(bucket_data);  
    }

    // fprintf(fp, "   ]\n}\n");
    fclose(fp);
    sprintf(resp, "Data from disk written to %s", SAVE_FILE);
ret:
    send_info_to_user(resp);
    return res;
}

char* get_kv_from_bucket(Entry *node) {
    // return all key-values in the bucket in a string form
    // k1:v1|expiry, k2:v2|expiry, ..., kn:vn|expiry
    char *data = calloc(1,KV_LEN*(ht->count+1));
    if (!data)return NULL;
    int data_idx = 0;
    int idx;
    while (node!=NULL) {
        idx = 0;
        while(node->key[idx]!='\0') data[data_idx++] = node->key[idx++];
        data[data_idx++] = '!';
        idx = 0;
        while(node->value[idx]!='\0') data[data_idx++] = node->value[idx++];
        data[data_idx++] = '!';
        char expiry[20];
        if (node->expiry!=ULONG_MAX)
            sprintf(expiry, "%zu", node->expiry);
        else
            sprintf(expiry, "-1");
        idx = 0;
        while (expiry[idx]!='\0') data[data_idx++] = expiry[idx++];
        // | for separating KV's in same bucket
        data[data_idx++] = '|';
        node = node->next;
    }
    data[data_idx-1] = '\0';
    return data;
}
