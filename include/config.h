#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"

// --configfile=
#define MIN_CONFIG_LEN 13
#define CONFIG_PREFIX "--configfile="
#define DEFAULT_SERVER_CONFIG_FILE "sahdb.conf"

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

/*
This log serves like an append file for every CRUD op performed
NO_LOG      -> 0 (No log updation done for WRITE op)
LOG_ONCE    -> 1 (Update the log every sec if WRITE op performed) 
LOG_ALL     -> 2 (Update the log after every WRITE op)
*/
typedef enum crud_op_log_level {
    NO_LOG,
    LOG_ONCE,
    LOG_ALL
} crud_op_log_level;
// Server config
typedef struct server_config {
    log_level   server_log_level;   // Level of the server log
    char        *server_log_file;   // Path of the server log
    char        *config_file;       // Path of --configfile
    int         server_port;        // Port to run server on, NULL if non-server
    char        *savefile;          // Path of the dump file (SAVE related)
    bool        rebuild;            // Flag to build from savefile
} server_config;

// extern server_config host_config;

// typedef struct node_config {
//     log_level node_log_level;
// } node_config;



typedef struct  {
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
