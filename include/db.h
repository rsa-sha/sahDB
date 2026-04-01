#ifndef DB_H
#define DB_H

#include "common.h"
#include "command.h"
#include "config.h"

err_t handle_request(conn_t *conn, char *req);
err_t getAndProcessCommand();
err_t processCommand(char *req, cmd_ctx *ctx);

extern server_config host_config;
extern commandNode commands[];

#endif
