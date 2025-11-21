#include "network.h"


void run_c_server(int port) {
    // server FD
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket failed");
        return;
    }

    struct sockaddr_in addr = {
        .sin_family         = AF_INET,
        .sin_addr.s_addr    = INADDR_ANY,
        .sin_port           = htons(port)
    };

    if(bind(server_fd, (struct sockaddr *)&addr, sizeof(addr))==-1) {
        perror("Bind failed");
        return;
    }
    if (listen(server_fd, MAX_REQ_IN_Q)) {
        perror("Listen Failed");
        goto ret;
    }
    char *resp = calloc(MAX_RESP_LEN, sizeof(char));
    if (!resp){
        perror("Memory allcation failure");
        goto ret;
    }

    sprintf(resp, "C server running on port %d", port);
    send_info_to_user(resp);
    free(resp);
    resp = NULL;

    err_t result = 0;
    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
            perror("accept failed");
            continue;
        }
        char buf[MAX_CMD_LEN];
        // update config for this client
        init_app_config(client_fd, client_fd);
        while (1) {
            /* code */
            if (get_user_input(buf)) break;
            result = processCommand(buf);
            if (result == DB_ERR_EXIT) {
                char *exiting = calloc(MAX_RESP_LEN, sizeof(char));
                if (!exiting){
                    perror("Memory allcation failure");
                    close(client_fd);
                    goto ret;
                }
                sprintf(exiting, "C server exiting");
                send_info_to_user(exiting);
                free(exiting);
                close(client_fd);
                goto ret;

            }
        }
        // close(client_fd);
        // break;
    }
ret:
    close(server_fd);
}

void eventLoop(){
    err_t res = 0;
    while (res!=DB_ERR_EXIT) {
        res = getAndProcessCommand();
    }
    return;
}

int main(int argc, char** argv) {
    ht_init();
    ttl_init();
    if(ht == NULL)
        return -1;
    // ./exec --port XXXX
    if (argc == 3) {
        // running on network?
        if (strcmp(argv[1], "--port")==0) {
            int port = atoi(argv[2]);
            run_c_server(port);
        }
        // rebuilding from file?
        if (strcmp(argv[1], "--savefile")==0) {
            rebuild_from_savefile();
            eventLoop();
        }
    }
    else
        eventLoop();
    return 0;
}