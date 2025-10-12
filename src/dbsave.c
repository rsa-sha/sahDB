#include "dbsave.h"

err_t fetch_dataset_from_memory(){
    err_t res = 0;
    FILE *fp = fopen(SAVE_FILE, "w");
    char resp[MAX_RESP_LEN] = {0};
    if (!fp){
        sprintf(resp, "Error opening %s file", SAVE_FILE);
        res = -1;
        goto ret;
    }
    // Write metadata
    fprintf(fp, "{\n");
    fprintf(fp, "   \"count\": %ld,\n", ht->count);
    fprintf(fp, "   \"size\": %ld,\n", ht->size);
    fprintf(fp, "   \"data\": [\n");
    // int n_buckets = 0;
    // char dataset[ht->count*KV_LEN];
    for(int i=0;i<ht->size;i++){
        char* bucket_data = get_kv_from_bucket(ht->buckets[i]);
        if(bucket_data && strlen(bucket_data)>0)
            fprintf(fp, "       {\"bucket\":%d, \"entries\":%s},\n", i, bucket_data);
        free(bucket_data);  
    }

    fprintf(fp, "   ]\n}\n");
    fclose(fp);
    sprintf(resp, "Data from disk written to %s", SAVE_FILE);
ret:
    send_info_to_user(resp);
    return res;
}

char* get_kv_from_bucket(Entry *node){
    // return all key-values in the bucket in a string form
    // k1:v1, k2:v2, ..., kn:vn
    char *data = calloc(1,KV_LEN*(ht->count+1));
    if(!data)return NULL;
    int data_idx = 0;
    int idx = 0;
    while(node!=NULL){
        idx = 0;
        while(node->key[idx]!='\0') data[data_idx++] = node->key[idx++];
        data[data_idx++] = ':';
        idx = 0;
        while(node->value[idx]!='\0')data[data_idx++] = node->value[idx++];
        data[data_idx++] = ',';
        node = node->next;
    }
    data[data_idx-1] = '\0';
    return data;
}
