#include "common.h"
#include "config.h"

int nl = 1;
// IO_Related Methods
err_t get_user_input(char *buf){
    if (!buf)return 1;
    nl = 0;
    send_info_to_user("sahDB> ");
    nl = 1;
    memset(buf, 0, MAX_CMD_LEN);
    // printf("sahDB> ");
    ssize_t bytes_read = read(app_config.input_fd, buf, MAX_CMD_LEN - 1);
    if (bytes_read < 1)
        return 1;
    if (buf[bytes_read - 1] == '\n')
        buf[bytes_read - 1] = '\0';
    else
        buf[bytes_read] = '\0';
    // if (fgets(buf, MAX_CMD_LEN, stdin) != NULL){
    //     return 0;
    // }
    // return 1;
    return 0;
}

// changed to write to given stream
void send_info_to_user(const char *data) {
    if (!data)
        return;
    ssize_t n = write(app_config.output_fd, data, strlen(data));
    if (n<0)
        perror("write");
    if (nl)
        write(app_config.output_fd, "\n", 1);
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