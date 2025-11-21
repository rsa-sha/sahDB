#ifndef DBSAVE_H
#define DBSAVE_H

#include "common.h"
#include "hash.h"

#define META_LEN 50
#define KV_LEN 1024
#define SAVE_FILE "data.safe"
#define FILE_HEADER_VER "SahDB001"
#define FILE_HEADER_LEN 8

extern HashTable* ht;

// methods for SAVE cmd
err_t fetch_dataset_from_memory();
char* get_kv_from_bucket(Entry *);
// err_t write_data_to_file();

err_t rebuild_from_savefile();
#endif