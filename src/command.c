#include "command.h"
#include "common.h"

// Page
Page p;

/*############################################
############### Commands Begin ###############
############################################*/

err_t exit_helper(int argc, char** cmd_arr){
    char resp[MAX_RESP_LEN];
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
    res = set_kv(key, val);
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
    res = get_kv(key);
ret:
    return res;
}

// HELP CMD methods
void print_commands(){
    char resp[MAX_RESP_LEN];
    char set[100];
    sprintf(set, "%sSET%s - %s%skey val%s", BOLD, RESET, BOLD, GREEN, RESET);
    char get[100];
    sprintf(get, "%sGET%s - %s%skey%s", BOLD, RESET, BOLD, GREEN, RESET);
    char help[100];
    sprintf(help, "%sHELP%s", BOLD, RESET);
    sprintf(resp, "%s\n%s\n%s", help, set, get);
    send_info_to_user(resp);
}

err_t help_helper(int argc, char** cmd_arr){
    // this method prints out commands and usage
    // if(argc==0){
    print_commands();
    // } else if(argc == 2 && strcmp(cmd_arr[1],"")!=0){
    //     printSubCommands(cmd_arr[1]);
    // }
    return 0;
}

/*############################################
################ Commands end ################
############################################*/

//TODO: build Commands array trees
commandNode commands[] = {
    {"HELP", NULL, true, 0, help_helper},
    {"SET", NULL, true, 0, set_key_val_helper},
    {"GET", NULL, true, 0, get_val_from_key_helper},
    {"EXIT", NULL, true, 0, exit_helper},
};
