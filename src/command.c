#include "command.h"
#include "common.h"
#include "hash.h"
#include "dbsave.h"

// Page
Page p;

/*############################################
############### Commands Begin ###############
############################################*/

err_t exit_helper(int argc, char** cmd_arr){
    char resp[MAX_RESP_LEN];
    if(argc!=1){
        char err[100];
        sprintf(err, "%sIncorrect number of args passed, run HELP EXIT%s", RED, RESET);
        send_info_to_user(err);
        return -1;
    }
    sprintf(resp, "%s%sExiting ...%s", RED, BOLD, RESET);
    send_info_to_user(resp);
    return SIG_EXIT;
}


// ########### SET command ###########
err_t set_kv(char* key, char* val){
    // if full raise Warning
    char resp[MAX_RESP_LEN];
    err_t ret = 0;
    // check if the key is already present so we have to update the value
    for(int i=0;i<p.num_records;i++){
        if (strcmp(key, p.records[i].key)==0){
            sprintf(resp, "Updated value of key %s to %s in db", key, val);
            strcpy(p.records[i].val, val);
            // p.records[i].val = val;
            goto ret;
        }
    }
    if(p.num_records == MAX_RECORDS){
        sprintf(resp, "%sPage already full, cannot add more data%s", RED, RESET);
        ret = 1;
        goto ret;
    }
    // is a new KV pair
    KV kp;
    strcpy(kp.key, key);
    strcpy(kp.val, val);
    p.records[p.num_records++] = kp;
    sprintf(resp, "Added key %s and value %s in DB", key, val);
ret:
    send_info_to_user(resp);
    return ret;
}

err_t set_key_val_helper(int argc, char** cmd_arr){
    err_t res = 0;
    if(argc!=3){
        char err[MAX_RESP_LEN];
        sprintf(err,"%sIncorrect number of args passed, run HELP SET%s", RED, RESET);
        send_info_to_user(err);
        res = -1;
        goto ret;
    }
    char *key = cmd_arr[1];
    char *val = cmd_arr[2];
    // res = set_kv(key, val);
    res = hash_insert(key, val);
ret:
    return res;
}


// ########### GET command ###########
err_t get_kv(char* key){
    // if full raise Warning
    char resp[MAX_RESP_LEN];
    err_t ret = 0;
    // check if the key is present so we have to print the value
    for(int i=0;i<p.num_records;i++){
        if (strcmp(key, p.records[i].key)==0){
            sprintf(resp, "%s",p.records[i].val);
            goto ret;
        }
    }
    sprintf(resp, "%sValue corresponding to key %s not present in DB%s", RED, key, RESET);
    ret = 1;
ret:
    send_info_to_user(resp);
    return ret;
}

err_t get_val_from_key_helper(int argc, char **cmd_arr){
    err_t res = 0;
    if(argc!=2){
        char err[100];
        sprintf(err, "%sIncorrect number of args passed, run HELP GET%s", RED, RESET);
        send_info_to_user(err);
        res = -1;
        goto ret;
    }
    char *key = cmd_arr[1];
    // res = get_kv(key);
    res = hash_get(key);
ret:
    return res;
}

// for use in help, defined in last of file
extern commandNode commands[];
// HELP CMD methods
err_t help(){
    char resp[MAX_RESP_LEN];
    int resp_idx = 0;
    int idx;
    for(int j=0; j<N_COMMANDS; j++){
        idx = 0;
        char *cmd = commands[j].name;
        while(cmd[idx]!='\0')
            resp[resp_idx++] = cmd[idx++];
        resp[resp_idx++] = '\t';
        idx = 0;
        char *usage = commands[j].usage;
        while(usage[idx]!='\0')
            resp[resp_idx++] = usage[idx++];
        resp[resp_idx++] = '\n';
    }
    // multiple endline fix
    resp[resp_idx-1] = '\0';
    send_info_to_user(resp);
    return 0;
}

err_t printSubCommands(char *arg){
    char resp[50];
    sprintf(resp, "%sNot support subcmd print for %s yet%s", RED, arg, RESET);
    send_info_to_user(resp);
    return 0;
}

err_t help_helper(int argc, char** cmd_arr){
    // this method prints out commands and usage
    if(argc==1){
        help();
    } else if(argc == 2 && strcmp(cmd_arr[1],"")!=0){
        printSubCommands(cmd_arr[1]);
    }
    return 0;
}

// EXISTS CMD
err_t exists_helper(int argc, char** cmd_arg){
    // check if key is present in the page
    char resp[100];
    err_t res = 0;
    if(argc!=2){
        sprintf(resp, "%sIncorrect number of args passed, run HELP EXISTS%s", RED, RESET);
        send_info_to_user(resp);
        res = -1;
        goto ret;
    }
    char *key = cmd_arg[1];
    // for(int i=0;i<p.num_records;i++){
    //     if( strcmp(p.records[i].key, key) == 0){
    //         sprintf(resp, "%s%sTRUE%s", BOLD, GREEN, RESET);
    //         goto ret;
    //     }
    // }
    // sprintf(resp, "%s%sFALSE%s", BOLD, RED, RESET);
    // res = 1;
    res = hash_exists(key);
ret:
    return res;
}

// DELETE CMD
err_t delete_key_value_helper(int argc, char** cmd_arg){
    // check if key is present in the page
    char resp[100];
    err_t res = 0;
    if(argc!=2){
        sprintf(resp, "%sIncorrect number of args passed, run HELP DELETE%s", RED, RESET);
        send_info_to_user(resp);
        res = -1;
        goto ret;
    }
    char *key = cmd_arg[1];
    res = hash_delete(key);
ret:
    return res;
}

// SAVE CMD
err_t save_helper(int argc, char** cmd_arg){
    char resp[100];
    err_t res = 0;
    if(argc!=1){
        sprintf(resp, "%sIncorrect number of args passed, run HELP SAVE%s", RED, RESET);
        send_info_to_user(resp);
        res = -1;
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
    {"SAVE", NULL, true, 0, save_helper,
        "Save the data from dataset to file         : " BOLD GREEN "SAVE" RESET
    },
    {"EXIT", NULL, true, 0, exit_helper,
        "EXIT the db process                        : " BOLD RED "EXIT" RESET
    },
};
