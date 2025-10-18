#ifndef DB_H
#define DB_H

#include "common.h"
#include "command.h"

err_t getAndProcessCommand();
err_t processCommand(char *req);

extern commandNode commands[];

#endif