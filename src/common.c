#include "common.h"
#include "config.h"

bool SILENT = false;
int nl = 1;
// IO_Related Methods
err_t get_user_input(char *buf) {
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
    return 0;
}

// For sharing information with user
void send_info_to_user(const char *data) {
    // do nothing if call from method for internal use
    if (SILENT)
        return;
    if (!data || strcmp(data, "")==0)
        return;
    ssize_t n = write(app_config.output_fd, data, strlen(data));
    if (n<0)
        perror("write");
    if (nl)
        write(app_config.output_fd, "\n", 1);
}

err_t tokenize(char *in, char **tokens) {
    int count = 0;
    int maxTokens = MAX_CMD_PARAMS;
    char *token = strtok(in, " \t\n");
    while (token!=NULL && count < maxTokens) {
        tokens[count++] = token;
        token = strtok(NULL, " \t\n");
    }
    return count; //excluding \0
}

// size in bytes using fseek and ftell
long get_file_size(const char *path) {
    FILE *fp = fopen(path, "r");
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fclose(fp);
    return size;
}


// network data write over socket
err_t socket_send_data(int socket, const char *fmt, ...) {
    err_t res = DB_ERR_OK;
    char resp[MAX_RESP_LEN];
    char buffer[MAX_RESP_LEN];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, MAX_RESP_LEN, fmt, args);
    va_end(args);
    ssize_t bytes_sent = send(socket, buffer, strlen(buffer), 0);
    if (bytes_sent < 1) {
        // unable to send data to the machine
        res = DB_ERR_GENERIC_FAIL;
        // this should be warning in log
        sprintf(resp, "Not able to share data to machine over network");
        send_info_to_user(resp);
    }
    return res;
}

// network data read over socket
err_t socket_read_data(int socket, char *buf, int bufsize) {
    err_t res = DB_ERR_OK;
    char resp[MAX_RESP_LEN];
retry:
    ssize_t bytes_read = read(socket, buf, bufsize);
    if (bytes_read > 0) {
        buf[bytes_read] = '\0';   // ← REQUIRED
        char *nl = strchr(buf, '\n');
        if (nl)*nl = '\0';
        goto ret;
    }
    if (bytes_read == 0) {
        // EOF
        res = DB_ERR_CLIENT_CLOSED;
        goto ret;
    }
    if (errno == EINTR) {
        goto retry;   // interrupted → try again
    }
    res = DB_ERR_CLIENT_FATAL;
ret:
    return res;
}


err_t user_socket_send_data(int socket, const char *buf, int ret_code) {
    // first of all we send the data string from server
    err_t res = DB_ERR_OK;
    res = socket_send_data(socket, "%s", buf);
    // send signal
    char ret_code_str[20];
    sprintf(ret_code_str, "%s%d", USER_SIG_HDR, ret_code);
    res = socket_send_data(socket, "%s", ret_code_str);
    return res;
}
