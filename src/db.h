#ifndef DB_H
#define DB_H

#include "common.h"
#include "command.h"

err_t getAndProcessCommand();

extern commandNode commands[];

#endif