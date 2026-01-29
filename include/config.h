#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"

// --configfile=
#define MIN_CONFIG_LEN 13
#define CONFIG_PREFIX "--configfile="
#define DEFAULT_SERVER_CONFIG_FILE "sahdb.conf"

// Network stuff (now for replication)
#define MIN_PORT    3000
#define MAX_PORT    65535

/* 
Log config
LOG_INFO    -> 0
LOG_DEBUG   -> 1
LOG_EXCEPT  -> 2
LOG_TRACE   -> 3
*/
typedef enum log_level {
    LOG_INFO,
    LOG_DEBUG,
    LOG_EXCEPT,
    LOG_TRACE
} log_level;

// State of replica/user connections with Master 
typedef enum conn_state_t {
    STATE_INACTIVE = -1,            // Node not responding
    STATE_HANDSHAKE = 0,            // Node initiating connection
    STATE_ACTIVE = 1,               // Node is ready/listening
} conn_state_t;

#define USER_PROMPT     "sahDB> "
#define USER_PROMPT_LEN 7

typedef enum conn_type_t {
    CONN_UNKN = -1,                 // Initialised value to the connection_t.conn_type field
    CONN_USER = 0,                  // Associated FD is of a User [eg. CLI/NC for CRUD]
    CONN_REPL = 1,                  // Associated FD is of a replica
} conn_type_t;

typedef struct conn_t {
    int                 fd;
    conn_type_t         conn_type;
    conn_state_t        conn_state;
} conn_t;

// Initial size allocated to connection_table array
#define DEF_CONN_TABLE_SIZE 32
typedef struct connection_table {
    conn_t      *conns;             // Connections array
    int         len;                // Current max size of array
    int         cap;                // Size of connections array
} connection_table;

// Struct on replica for comm with master node
typedef struct master_info {
    char        *name;              // Conf param for connection with master
    bool        is_conn;            // True if connected to master
    int         sock;               // Used for comms bw nodes
    int         port;               // Port of the master on it's machine
} master_info;

// Struct on master for info of replica node
typedef struct replicaNode {
    char        *ip;            // IP of replica
    char        *hostname;      // hostname of replica
    int         port;           // master shares stream on this port to replica
    char        *uuid;          // For verification of node during replication
    struct replicaNode *next;   // Poninter to the next replicaNode
} replicaNode;

// Server config [in-mem info on the current node related to configuration and other nodes]
typedef struct server_config {
    log_level           server_log_level;   // Level of the server log
    char                *server_log_file;   // Path of the server log
    char                *config_file;       // Path of --configfile
    char                *ip;                // IP string associated to node, can be localhost or IP addr val    
    int                 server_port;        // Port to run server on, NULL if non-server
    char                *savefile;          // Path of the dump file (SAVE related)
    bool                rebuild;            // Flag to build from savefile
    bool                is_master_node;     // Flag for master role, only used in case of ntwk
    char                *uuid;              // For verification of node during replication
    int                 num_replicas;       // Number of replicas
    replicaNode         *replicas;          // For replication purposes
    master_info         *master;            // For master comms
    connection_table    *connections;       // For listening for requests from FDs
} server_config;

#define NUM_CONNS host_config.connections->len
#define CONN(I) host_config.connections->conns[I]

// Allocates memory for connection_table
err_t init_conn_table();

// Returns true if connection_table is filled 80% or more
#define CONN_TABLE_LIMIT \
    host_config.connections->len >= (host_config.connections->cap *8)/10
// Double connection_table size called at 80% full
err_t double_conn_table();

// Cleans up conn at index i and puts the last conn in current idx
err_t remove_entry_conn_table(size_t idx);

// Cleans up conns, called during server shutdown (assumes FDs, sockets closed)
err_t purge_conn_table();

// extern server_config host_config;

// typedef struct node_config {
//     log_level node_log_level;
// } node_config;



typedef struct AppConfig {
    int input_fd;
    int output_fd;
} AppConfig;

extern AppConfig app_config;

void init_app_config(int fd_in, int fd_out);

/*
This method builds up the internal config params based on the config file passed
on with the --configfile flag on start-up or else continues with the default file
*/
err_t init_server_config(int argc, char **argv);
#endif
