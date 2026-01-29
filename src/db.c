#include "db.h"
// #include "command.h"

err_t processCommand(char *req, int idx) {
    char **cmd_arr = malloc(sizeof(char)*(MAX_CMD_LEN*2));
    // split req into tokens
    err_t res = tokenize(req, cmd_arr);
    // TODO: tree walk for command check
    // compare the first entry of cmd_arr to get command type
    func_ptr handler = NULL;
    bool checkSubCmd = false;
    int i;
    for (i=0;i<N_COMMANDS;i++) {
        char *cmd = commands[i].name;
        // compare without case
        if ((cmd!=NULL) && (cmd_arr!=NULL) && (strcasecmp(cmd, cmd_arr[0]) == 0)) {
            handler = commands[i].func;
            if (commands[i].subcmds != NULL) {
                checkSubCmd = true;
            }
            break;
        }
    }
    if (i==N_COMMANDS && handler == NULL) {
        char resp[MAX_RESP_LEN];
        if (idx>-1) {
            conn_t *conn = &CONN(idx);
            sprintf(resp, "%s%sCommand %s not a supported CMD. Use HELP cmd to know more%s\n", BOLD, YELLOW, cmd_arr[0], RESET);
            socket_send_data(conn->fd, resp);
            // move ptr to next line
        } else {
            sprintf(resp, "%s%sCommand %s not a supported CMD. Use HELP cmd to know more%s", BOLD, YELLOW, cmd_arr[0], RESET);
            send_info_to_user(resp);
        }
        res = DB_ERR_CMD_NOTEXIST;
        goto ret;
    }
    if (checkSubCmd) {
        // process subcmd check
    }
    // if works call func-ptr for processing
    res = handler(res, cmd_arr);
ret:
    return res;
}

// this should only run in non server mode 
err_t getAndProcessCommand() {
    char cmd[MAX_CMD_LEN];
    err_t res = get_user_input(cmd);
    // cmd string is null|escape seq.
    if (cmd[0]=='\0' || isalnum(cmd[0])==0 || res) {
        char output[MAX_RESP_LEN];
        sprintf(output, "%s%sInvalid operation requested%s", BOLD, RED, RESET);
        send_info_to_user(output);
        goto ret;
    } else {
        // we'll process cmd
        res = processCommand(cmd, -1);
    }
ret:
    return res;
}