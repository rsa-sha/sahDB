#include "cli.h"

cli_s cli;

err_t is_port(char *st) {
    int i=0;
    while(st[i]!='\0') {
        if(!isdigit(st[i]))
            goto err_ret;
        i++;
    }
    int port = atoi(st);
    if (port<1000 || port>65535)
        goto err_ret;
    cli.port = port;
ret:
    return DB_ERR_OK;
err_ret:
    strcpy(cli.resp, "Invalid port number");
    return 1;
}

char *usage_str() {
    return "--port|-p [PORT] "\
    "--host|-h [IP of server; localhost by default}]";
}

err_t init_cli_config(int argc, char **argv) {
    int i=0, res = DB_ERR_OK;
    while(i<argc) {
        if ((strcasecmp(argv[i], "--port")==0 || strcasecmp(argv[i], "-p")==0) && i+1<argc) {
            if(is_port(argv[i+1]) != DB_ERR_OK){
                res = DB_ERR_INVALID_ARGS;
                goto ret;
            }
            cli.port = atoi(argv[i+1]);
            i++;
        } else if ((strcasecmp(argv[i], "--host")==0 || strcasecmp(argv[i], "-h")==0) && i+1<argc) {
            cli.ip = argv[++i];
            cli.local = false;
        }
        i++;
    }

ret:
    return res;
}

err_t handshake_with_server() {
    struct sockaddr_in server;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        strcpy(cli.resp, "socket() failed");
        // should be DB_ERR_NETWORK
        return DB_ERR_GENERIC_FAIL;
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(cli.port);
    if (inet_pton(AF_INET, cli.ip, &server.sin_addr) != 1) {
        strcpy(cli.resp, "Invalid server IP");
        close(sock);
        return DB_ERR_INVALID_ARGS;
    }
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        strcpy(cli.resp, "Connection failed");
        close(sock);
        // should be DB_ERR_NETWORK
        return DB_ERR_GENERIC_FAIL;
    }
    // Handshake with server: send USER_JOIN
    int res = send(sock, HANDSHAKE_INIT, HANDSHAKE_INIT_LEN, 0);
    if (res != HANDSHAKE_INIT_LEN) {
        strcpy(cli.resp,  "Unable to start connection with server");
        close(sock);
        // should be DB_ERR_NETWORK
        return DB_ERR_GENERIC_FAIL;
    }
    cli.sockfd = sock;
    cli.server = server;
    return DB_ERR_OK;
}

err_t process_user_req() {
    char cmd[MAX_CMD_LEN];
    char resp[MAX_RESP_LEN];
    //SILENT = ~SILENT;
    get_user_input(cmd);
    if(cmd[0] == '\n' || cmd[0] == '\0'){
        return DB_ERR_OK;
    }
    int sz = strlen(cmd);
    int retries = 3;
    int res;
    bool failure = true;
    //SILENT = ~SILENT;
    // retry loop for user request
    while(retries-- && (res = send(cli.sockfd, cmd, sz, 0))) {
        if (res==sz) {
            failure = false;
            break;
        }
    }
    if (failure){
        strcpy(cli.resp, "Failed to send request to server");
        // Something like DB_ERR_SERVER_REQ_FAIL
        send_info_to_user(cli.resp);
        return DB_ERR_GENERIC_FAIL;
    }
    if(recv(cli.sockfd, resp, MAX_RESP_LEN, 0)==-1) {
        strcpy(cli.resp, "Did not receive response from server for request");
        // Something like DB_ERR_SERVER_REQ_FAIL
        send_info_to_user(cli.resp);
        return DB_ERR_GENERIC_FAIL;
    }
    printf("output_fd = %d\n", app_config.output_fd);
    send_info_to_user(resp);
    return DB_ERR_OK;
}

void dump_cli_conf_to_term() {
    printf("cli_s struct:\n");
    printf("\tlocal: \t%s\n", cli.local?"true":"false");
    printf("\tIP: \t%s\n", cli.ip);
    printf("\tPort: \t%d\n", cli.port);
}

int main(int argc, char **argv) {
    cli.local = true;
    cli.resp[0] = '\0';
    err_t ret_code = DB_ERR_OK;
    if (argc < 2) {
        printf("Usage: %s %s\n", argv[0], usage_str());
        return DB_ERR_INVALID_ARGS;
    }
    if ((ret_code = init_cli_config(argc, argv))!=DB_ERR_OK)return ret_code;
    if(cli.port == 0) {
        strcpy(cli.resp, "Invalid port number");
        ret_code = DB_ERR_INVALID_ARGS;
        goto ret;
    }
    if (cli.local==true)cli.ip = "127.0.0.1";

    // initiate handshake with server
    ret_code = handshake_with_server();
    if (ret_code!=DB_ERR_OK) {
        // Handshake failed
        // send err resp to user
    }
    while(ret_code!=DB_ERR_EXIT && ret_code !=DB_ERR_SHUTDOWN){
        ret_code = process_user_req();
    } 
    dump_cli_conf_to_term();
ret:
    if (cli.resp[0]!='\0')
        send_info_to_user(cli.resp);
    return ret_code;
}
