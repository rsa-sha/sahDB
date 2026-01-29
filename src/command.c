#include "command.h"
#include "common.h"
#include "hash.h"
#include "dbsave.h"

// Page
Page p;

/*############################################
############### Commands Begin ###############
############################################*/

// This method terminates the conn from USER or REPLICA
err_t exit_helper(int argc, char **cmd_arr) {
    char resp[MAX_RESP_LEN];
    if (argc!=1) {
        char err[100];
        sprintf(err, "%sIncorrect number of args passed, run HELP EXIT%s", RED, RESET);
        send_info_to_user(err);
        return DB_ERR_INVAILD_ARGS;
    }
    sprintf(resp, "%s%sExiting ...%s", RED, BOLD, RESET);
    send_info_to_user(resp);
    return DB_ERR_EXIT;
}

// This method will end server
err_t shutdown_helper(int argc, char **cmd_arr) {
    char resp[MAX_RESP_LEN];
    if (argc!=1) {
        char err[100];
        sprintf(err, "%sIncorrect number of args passed, run HELP EXIT%s", RED, RESET);
        send_info_to_user(err);
        return DB_ERR_INVAILD_ARGS;
    }
    sprintf(resp, "%s%sShutting down server ...%s", RED, BOLD, RESET);
    send_info_to_user(resp);
    return DB_ERR_SHUTDOWN;
}


// ########### SET command ###########
err_t set_key_val_helper(int argc, char **cmd_arr) {
    err_t res = 0;
    if (argc!=3) {
        char err[MAX_RESP_LEN];
        sprintf(err,"%sIncorrect number of args passed, run HELP SET%s", RED, RESET);
        send_info_to_user(err);
        res = DB_ERR_INVAILD_ARGS;
        goto ret;
    }
    char *key = cmd_arr[1];
    char *val = cmd_arr[2];
    res = hash_insert(key, val);
ret:
    return res;
}


// ########### GET command ###########
err_t get_val_from_key_helper(int argc, char **cmd_arr) {
    err_t res = 0;
    if (argc<2) {
        char err[100];
        sprintf(err, "%sIncorrect number of args passed, run HELP GET%s", RED, RESET);
        send_info_to_user(err);
        res = DB_ERR_INVAILD_ARGS;
        goto ret;
    }
    if (argc == 2) {
        char *key = cmd_arr[1];
        res = hash_get(key);
    }
    if (argc == 3 && !strcasecmp(cmd_arr[2], "ex")) {
        char *key = cmd_arr[1];
        res = hash_get_expiry(key);
    }
ret:
    return res;
}

// for use in help, defined in last of file
extern commandNode commands[];
// HELP CMD methods
err_t help() {
    char resp[MAX_RESP_LEN];
    size_t resp_idx = 0;
    size_t max_cmd_len = 0;
    size_t max_desc_len = 0;
    /* 1) Compute column widths */
    for (int i = 0; i < N_COMMANDS; i++) {
        const char *cmd = commands[i].name;
        const char *usage = commands[i].usage;
        const char *colon = strchr(usage, ':');
        size_t desc_len = colon ? (size_t)(colon - usage) : strlen(usage);
        max_cmd_len  = MAX(max_cmd_len, strlen(cmd));
        max_desc_len = MAX(max_desc_len, desc_len);
    }
    /* 2) Format output */
    for (int i = 0; i < N_COMMANDS; i++) {
        const char *cmd = commands[i].name;
        const char *usage = commands[i].usage;
        const char *colon = strchr(usage, ':');
        const char *desc = usage;
        const char *syntax = colon ? colon + 1 : "";
        size_t desc_len = colon ? (size_t)(colon - usage) : strlen(usage);
        int written = snprintf(
            resp + resp_idx,
            MAX_RESP_LEN - resp_idx,
            "%-*s\t%-*.*s :%s\n",
            (int)max_cmd_len, cmd,
            (int)max_desc_len, (int)desc_len, desc,
            syntax
        );
        if (written < 0 || resp_idx + written >= MAX_RESP_LEN)
            break;
        resp_idx += written;
    }
    /* Remove final newline */
    if (resp_idx > 0)
        resp[resp_idx - 1] = '\0';
    send_info_to_user(resp);
    return 0;
}

err_t printSubCommands(char *arg) {
    char resp[50];
    sprintf(resp, "%sNot support subcmd print for %s yet%s", RED, arg, RESET);
    send_info_to_user(resp);
    return 0;
}

err_t help_helper(int argc, char **cmd_arr) {
    // this method prints out commands and usage
    if (argc==1) {
        help();
    } else if (argc == 2 && strcmp(cmd_arr[1],"")!=0) {
        printSubCommands(cmd_arr[1]);
    }
    return 0;
}

// EXISTS CMD
err_t exists_helper(int argc, char **cmd_arr) {
    // check if key is present in the page
    char resp[100];
    err_t res = 0;
    if (argc!=2) {
        sprintf(resp, "%sIncorrect number of args passed, run HELP EXISTS%s", RED, RESET);
        send_info_to_user(resp);
        res = DB_ERR_INVAILD_ARGS;
        goto ret;
    }
    char *key = cmd_arr[1];
    res = hash_exists(key);
ret:
    return res;
}

// DELETE CMD
err_t delete_key_value_helper(int argc, char **cmd_arr) {
    // check if key is present in the page
    char resp[100];
    err_t res = 0;
    if (argc!=2) {
        sprintf(resp, "%sIncorrect number of args passed, run HELP DELETE%s", RED, RESET);
        send_info_to_user(resp);
        res = DB_ERR_INVAILD_ARGS;
        goto ret;
    }
    char *key = cmd_arr[1];
    res = hash_delete(key);
ret:
    return res;
}

// EXPIRE CMD
err_t expire_key_val(int argc, char **cmd_arr) {
    char resp[100];
    err_t res = 0;
    if (argc!=3) {
        sprintf(resp, "%sIncorrect number of args passed, run HELP EXPIRE%s", RED, RESET);
        res = DB_ERR_INVAILD_ARGS;
        send_info_to_user(resp);
        goto ret;
    }
    char *key = cmd_arr[1];
    long expiry = (long)atol(cmd_arr[2]);
    SILENT = true;
    int key_exists_in_ht = hash_exists(key);
    SILENT = false;
    if (key_exists_in_ht == 0)
        res = hash_update_expiry(key, expiry);
ret:
    return res;
}

// SAVE CMD
err_t save_helper(int argc, char **cmd_arr) {
    char resp[100];
    err_t res = 0;
    if (argc!=1) {
        sprintf(resp, "%sIncorrect number of args passed, run HELP SAVE%s", RED, RESET);
        send_info_to_user(resp);
        res = DB_ERR_INVAILD_ARGS;
        goto ret;
    }
    res = fetch_dataset_from_memory();
ret:
    return res;
}

/*############################################
################ Commands end ################
############################################*/

//TODO: build Commands array trees
commandNode commands[] = {
    {"HELP", NULL, true, 0, help_helper,
        "Display the help menu                      : " BOLD GREEN "HELP" RESET
    },
    {"SET", NULL, true, 0, set_key_val_helper,
        "Add/Update Key-Value pair in the dataset   : " BOLD GREEN "SET key val" RESET
    },
    {"EXISTS", NULL, true, 0, exists_helper,
        "Check if a key exits in dataset            : " BOLD GREEN "EXISTS key" RESET
    },
    {"GET", NULL, true, 0, get_val_from_key_helper,
        "Print the value corresponding to a key     : " BOLD GREEN "GET key" RESET
    },
    {"DELETE", NULL, true, 0, delete_key_value_helper,
        "Remove Key-Value pair from the dataset     : " BOLD RED "DELETE key" RESET
    },
    {"EXPIRE", NULL, true, 0, expire_key_val,
        "Add/Update expuration time of KV pair      : " BOLD RED "EXPIRE key time_in_sec" RESET
    },
    {"SAVE", NULL, true, 0, save_helper,
        "Save the data from dataset to file         : " BOLD GREEN "SAVE" RESET
    },
    {"EXIT", NULL, true, 0, exit_helper,
        "Exit out of the User|Replica connection    : " BOLD RED "EXIT" RESET
    },
    {"SHUTDOWN", NULL, true, 0, shutdown_helper,
        "SHUTDOWN the DB server                     : " BOLD RED "SHUTDOWN" RESET
    },
};
