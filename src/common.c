#include "common.h"

// IO_Related Methods
err_t get_user_input(char *cmd){
    printf("sahDB> ");
    if (fgets(cmd, MAX_CMD_LEN, stdin) != NULL){
        return 0;
    }
    return 1;
}

void send_info_to_user(char *data){
    fflush(stdin);
    printf("%s\n", data);
}

err_t tokenize(char *in, char **tokens){
    int count = 0;
    int maxTokens = MAX_CMD_PARAMS;
    char *token = strtok(in, " \t\n");
    while(token!=NULL && count < maxTokens){
        tokens[count++] = token;
        token = strtok(NULL, " \t\n");
    }
    return count; //excluding \0
}