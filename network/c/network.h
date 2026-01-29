#ifndef NETWORK_H
#define NETWORK_H
#include "common.h"
#include "config.h"
#include "db.h"
#include "dbsave.h"
#include "hash.h"
#include "replication.h"
#include "ttl.h"

#include <netinet/in.h>

extern void run_c_server(int port);

extern HashTable *ht;
extern server_config host_config;

#define MAX_REQ_IN_Q    10
#define DEFAULT_PORT    5000
#endif