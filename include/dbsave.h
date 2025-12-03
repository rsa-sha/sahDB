#ifndef DBSAVE_H
#define DBSAVE_H

#include "common.h"
#include "config.h"
#include "hash.h"

extern server_config host_config;

#define META_LEN 50
#define KV_LEN 1024
#define DEFAULT_SAVE_FILE "data.safe"
#define SAVE_FILE host_config.savefile
#define FILE_HEADER_VER "SahDB001"
#define FILE_HEADER_LEN 8

extern HashTable* ht;

// methods for SAVE cmd
err_t fetch_dataset_from_memory();
char* get_kv_from_bucket(Entry *);
// err_t write_data_to_file();

err_t rebuild_from_savefile();
#endif