#include "config.h"
#include "common.h"
#include "dbsave.h"

AppConfig app_config;
server_config host_config;

// Trim leading and trailing whitespace in-place.
// Returns pointer to trimmed string start.
static char* trim(char *s) {
    char *end;

    // Trim leading spaces
    while (isspace((unsigned char)*s)) s++;
    // If string is all spaces, return empty string
    if (*s == 0)
        return s;
    // Trim trailing spaces
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    // Write terminating null
    *(end + 1) = '\0';
    return s;
}

void init_app_config(int fd_in, int fd_out) {
    app_config.input_fd = (fd_in > 0) ? fd_in : STDIN_FILENO;
    app_config.output_fd = (fd_out > 0) ? fd_in : STDOUT_FILENO;
}

static err_t load_server_config() {
    err_t res = DB_ERR_OK;
    char resp[MAX_RESP_LEN];
    FILE *fp = fopen(host_config.config_file, "r");
    if (fp == NULL) {
        res = DB_ERR_GENERIC_FAIL;
        sprintf(resp, "Unable to access file %s, setting to default", host_config.config_file);
        host_config.savefile = DEFAULT_SAVE_FILE;
        goto ret;
    }
    // keeping line to max 512 chars for now
    char line[MAX_CMD_LEN];
    while (fgets(line, MAX_CMD_LEN, fp)) {
        line[strcspn(line, "\n")] = '\0';
        // Find the position of '=' in the line
        char *equal_sign_pos = strchr(line, '=');
        if (equal_sign_pos != NULL) {
            *equal_sign_pos = '\0';  // Split the string at the '='
            char *field = trim(line);
            char *value = trim(equal_sign_pos + 1);
            char field_resp[MAX_RESP_LEN];
            // Print the field and value (can be stored in a dictionary here)
            sprintf(field_resp, "Field: '%s', Value: '%s'", field, value);
            if (strcasecmp(field, "port") == 0 && atoi(value) > 0) {
                host_config.server_port = atoi(value);
            } else if (strcasecmp(field, "savefile") == 0) {
                FILE *save_ptr = fopen(value, "r");
                if (save_ptr == NULL){
                    sprintf(resp, "Unable to access file %s, setting to default", value);
                    res = DB_ERR_GENERIC_FAIL;
                    host_config.savefile = DEFAULT_SAVE_FILE;
                    continue;
                }
                host_config.savefile = value;
                host_config.rebuild = true;
            } else if (strcasecmp(field, "master") == 0 && value ){
                // MASTER=...
                master_info *master = malloc(sizeof(master_info));
                if (master == NULL) {
                    send_info_to_user("Unable to alloc mem for master");
                    res = DB_ERR_NOMEM;
                    continue;
                }
                char *colon = strchr(value, ':');
                if (colon) {
                    *colon = '\0';
                    master->name = strdup(value);
                    master->is_conn = false;
                    master->sock = -1;
                    master->port = atoi(colon+1);
                    host_config.master = master;
                } else {
                    // incorrect config won't set master
                    send_info_to_user("Issue parsing master info, incorrect config");
                    free(master);
                    res = DB_ERR_INVAILD_ARGS;
                    continue;
                }

            } else {
                continue;
            }
            send_info_to_user(field_resp);
        } else if (strcasecmp(line, "ismaster") == 0) {
            host_config.is_master_node = true;
        }
    }
ret:
    send_info_to_user(resp);
    return res;
}

err_t init_server_config(int argc, char **argv) {
    err_t res = DB_ERR_OK;
    host_config.server_log_level = LOG_INFO;
    host_config.server_log_file = NULL;
    host_config.config_file = NULL;
    char resp[100];
    // conf file exists
    for(int i=0; i < argc; i++) {
        if (strlen(argv[i]) >= MIN_CONFIG_LEN) {
            if (strncmp(argv[i], CONFIG_PREFIX, MIN_CONFIG_LEN) == 0) {
                char *fname = strtok(argv[i], "=");
                fname = strtok(NULL, "");
                host_config.config_file = fname;
                load_server_config();
                sprintf(resp, "Config %s, loaded :)", host_config.config_file);
                send_info_to_user(resp);
            }
        }
        if (strcmp(argv[i], "--port") == 0 && i+1 < argc) {
            host_config.server_port = atoi(argv[i+1]);
        }
        if (strcmp(argv[i], "--savefile") == 0 && i+1 < argc) {
            FILE *save_ptr = fopen(argv[i+1], "r");
            if (save_ptr == NULL){
                sprintf(resp, "Not able to access file %s :(", argv[i+1]);
                send_info_to_user(resp);
                res = DB_ERR_GENERIC_FAIL;
                // setting save file to the default name
                // TODO: should I have this by default initially?? I feel no
                host_config.savefile = DEFAULT_SAVE_FILE;
                continue;
            }
            host_config.savefile = argv[i+1];
            host_config.rebuild = true;
        }
        if (strcmp(argv[i], "--ismaster") == 0) {
            host_config.is_master_node = true;
        }
    }
    // create attribute list from config file
    return res;
}

//////////////////////////////
// Connection table methods //
//////////////////////////////


err_t init_conn_table() {
    err_t res = DB_ERR_OK;
    char resp[100];
    connection_table *conn_table = calloc(1, sizeof(connection_table));
    if (conn_table==NULL) {
        sprintf(resp, "Failed to allocate memory for connection table");
        res = DB_ERR_NOMEM;
        goto ret;
    }
    conn_table->conns = calloc(DEF_CONN_TABLE_SIZE, sizeof(conn_t));
    if (conn_table->conns==NULL) {
        sprintf(resp, "Failed to allocate memory for connection table array");
        res = DB_ERR_NOMEM;
        goto ret;
    }
    conn_table->cap = DEF_CONN_TABLE_SIZE;
    conn_table->len = 0;
    host_config.connections = conn_table;
    sprintf(resp, "Connection table init complete");
ret:
    //TODO: has to be internal to log of current node
    send_info_to_user(resp);
    return res;
}


err_t double_conn_table() {
    err_t res = DB_ERR_OK;
    char resp[100];
    conn_t *updated_conns = realloc(
        host_config.connections->conns,
        (DEF_CONN_TABLE_SIZE * 2) * sizeof(conn_t));
    if (!host_config.connections->conns) {
        sprintf(resp, "Failed to allocate memory for doubling connection table array");
        res = DB_ERR_NOMEM;
        goto ret;
    }
    host_config.connections->conns = updated_conns;
    host_config.connections->cap = DEF_CONN_TABLE_SIZE * 2;
    host_config.connections->len = 0;
    sprintf(resp, "Connection table size doubled");
ret:
    //TODO: has to be internal to log of current node
    send_info_to_user(resp);
    return res;
}


err_t remove_entry_conn_table(size_t idx) {
    err_t res = DB_ERR_OK;
    char resp[100];
    if (idx >= host_config.connections->cap) {
        sprintf(resp, "Index > used size : SEG FAULT");
        res = DB_ERR_INVAILD_ARGS;
        goto ret;
    }
    size_t last = NUM_CONNS - 1;
    if (idx !=last)
        CONN(idx) = CONN(last);
    host_config.connections->len--;
        sprintf(resp, "Removed entry");
ret:
    //TODO: has to be internal to log of current node
    send_info_to_user(resp);
    return res;
}


err_t purge_conn_table() {
    // free connection array
    free(host_config.connections->conns);
    host_config.connections->cap = 0;
    host_config.connections->len = 0;
    // free connection table
    free(host_config.connections);
    return DB_ERR_OK;
}