#include "db.h"
// #include "command.h"

err_t processCommand(char *req){
    char **cmd_arr = malloc(sizeof(char)*(MAX_CMD_LEN*2));
    // split req into tokens
    err_t res = tokenize(req, cmd_arr);
    // TODO: tree walk for command check
    // compare the first entry of cmd_arr to get command type
    func_ptr handler = NULL;
    bool checkSubCmd = false;
    int i;
    for(i=0;i<N_COMMANDS;i++){
        char *cmd = commands[i].name;
        // compare without case
        if(strcasecmp(cmd, cmd_arr[0]) == 0){
            handler = commands[i].func;
            if (commands[i].subcmds != NULL){
                checkSubCmd = true;
            }
            break;
        }
    }
    if(i==N_COMMANDS && handler == NULL){
        char resp[MAX_RESP_LEN];
        sprintf(resp, "%s%sCommand %s not a supported CMD. Use HELP cmd to know more%s", BOLD, YELLOW, cmd_arr[0], RESET);
        send_info_to_user(resp);
        res = -1;
        goto ret;
    }
    if(checkSubCmd){
        // process subcmd check
    }
    // if works call func-ptr for processing
    res = handler(res, cmd_arr);
ret:
    return res;
}

err_t getAndProcessCommand(){
    char cmd[MAX_CMD_LEN];
    err_t res = get_user_input(cmd);
    // cmd string is null|escape seq.
    if(cmd[0]=='\0' || isalnum(cmd[0])==0 || res){
        char output[MAX_RESP_LEN];
        sprintf(output, "%s%sInvalid operation requested%s", BOLD, RED, RESET);
        send_info_to_user(output);
    } else {
        // we'll process cmd
        res = processCommand(cmd);
    }
ret:
    return res;
}