#include "network.h"

/*
Updated implementaiton, spilt up work of nodes
What stuff is covered as part of this func:
- Accept new connections
- Watches all active FDs (User-Cli | Replica-Server)
- Read only when there is data to process
- Never block on any proc (hope so) 
*/
void run_as_server() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        // TODO: this has to be in server.log
        send_info_to_user("Unable to provide socket for running the server");
        exit(DB_ERR_GENERIC_FAIL);
    }
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(host_config.server_port),
    };
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        // TODO: this has to be in server.log
        send_info_to_user("Unable to bind socket for running the server");
        close(server_fd);
        exit(DB_ERR_GENERIC_FAIL);
    }
    if (listen(server_fd, MAX_REQ_IN_Q) < 0) {
        // TODO: this has to be in server.log
        send_info_to_user("Unable to listen on the socket");
        close(server_fd);
        exit(DB_ERR_GENERIC_FAIL);
    }
    // TODO: this has to be in server.log
    send_info_to_user("Server started");

    // Init Connection_Table array
    if (init_conn_table()!=DB_ERR_OK)return;

    while (1) {
        // Realloc conn table if reached 80% capacity
        // if (CONN_TABLE_LIMIT && double_conn_table()!=DB_ERR_OK)return;

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int maxfd = server_fd;      // range of FDs to monitor
        // checking existing connections
        for (size_t i = 0; i < NUM_CONNS; i++) {
            conn_t *temp = &CONN(i);
            if (temp->fd != -1) {
                FD_SET(temp->fd, &readfds);
                if (temp->fd > maxfd)
                    maxfd = temp->fd;
            }
        }
        // reading the FDs
        int read = select(maxfd+1, &readfds, NULL, NULL, NULL);
        if (read < 0) {
            if (errno = EINTR) {
                // TODO: this has to be in server.log
                send_info_to_user("SELECT syscall got interrupted");
                break;
            }
        }
        // new conns
        if (FD_ISSET(server_fd, &readfds)) {
            int client_fd = accept(server_fd, NULL, NULL);
            // if (client_fd >= 0) {
            //     // add new conn to the list
            //     host_config.connections = realloc(
            //         host_config.connections,
            //         sizeof(connection_t) * (host_config.num_conns + 1)
            //     );
            //     if (!host_config.connections) {
            //         // TODO: this has to be in server.log
            //         send_info_to_user("Unable to allocate memory for the new connection");
            //         close(client_fd);
            //         continue;
            //     }
            // }
            // Initialising the connection
            CONN(NUM_CONNS).fd = client_fd;
            CONN(NUM_CONNS).conn_type = CONN_UNKN;
            CONN(NUM_CONNS).conn_state = STATE_HANDSHAKE;
            host_config.connections->len++;
        }

        // Existing connections
        for (int i = 0; i < NUM_CONNS; i++) {
            if (CONN(i).fd == -1 || !FD_ISSET(CONN(i).fd, &readfds))
                continue;
            char buf[MAX_CMD_LEN];
            if (socket_read_data(CONN(i).fd, buf, MAX_CMD_LEN - 1) !=DB_ERR_OK) {
                close(CONN(i).fd);
                CONN(i).fd = -1;
                continue;
            }
            /* HANDSHAKE */
            if (CONN(i).conn_state == STATE_HANDSHAKE) {
                if (strcmp(buf, USER_JOIN) == 0) {
                    CONN(i).conn_type = CONN_USER;
                    CONN(i).conn_state = STATE_ACTIVE;
                    send(CONN(i).fd, USER_PROMPT, USER_PROMPT_LEN, 0);
                } else if (strcmp(buf, REPL_JOIN) == 0) {
                    CONN(i).conn_type = CONN_REPL;
                    CONN(i).conn_state = STATE_ACTIVE;
                    master_init_handshake_with_repl(CONN(i).fd);
                    // TODO: do we need to update state of REPL??
                } else {
                    // invalid conn
                    close(CONN(i).fd);
                    CONN(i).fd = -1;
                }
                continue;
            }
            /* ACTIVE */
            if(CONN(i).conn_type == CONN_USER && CONN(i).fd > 0) {
                err_t res = processCommand(buf, i);
                if (res == DB_ERR_EXIT) {
                    close(CONN(i).fd);
                    CONN(i).fd = -1;
                } else if (res == DB_ERR_SHUTDOWN) {
                    goto shutdown_server;
                }
            }
            // TODO: Replica req handler
            // else if (CONN(i).conn_type == CONN_REPL) {
            //     handle_repl_message(CONN(i).fd, buf);
            // }
        }
    }
shutdown_server:
    // cleanup all structs
    purge_conn_table();
    close(server_fd);
}

/*
eventLoop func takes no args
Will only run when DB started in non-server mode locally
*/
void eventLoop(){
    err_t res = 0;
    while (res!=DB_ERR_SHUTDOWN) {
        res = getAndProcessCommand();
    }
    return;
}

int main(int argc, char** argv) {
    // initialize internal config variables
    init_server_config(argc, argv);
    ht_init();
    ttl_init();
    if(ht == NULL)
        return -1;
    if (host_config.rebuild) {
        rebuild_from_savefile();
        host_config.rebuild = false;
    }
    if (host_config.master != NULL && host_config.master->is_conn == false) {
        err_t temp = initialise_conn_with_master();
        if ( temp != DB_ERR_OK) {
            // we return after stopping everything
            // TODO: cleanall()
            send_info_to_user("Failed to conn with master");
            return temp; 
        } else {
            if (!host_config.server_port)
                host_config.server_port = 5050;
        }
    }
    if (host_config.server_port > 0) {
        run_as_server();
        // run_as_server(host_config.server_port);
    } else {
        eventLoop();
    }
    return 0;
}