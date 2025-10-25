#ifndef COMMON_H
#define COMMON_H

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define err_t int
#define ERR_FULL 100
// IO_Related Methods
#define MAX_CMD_LEN 512
#define MAX_CMD_PARAMS 16
#define MAX_RESP_LEN 8192
err_t get_user_input(char *buf);

void send_info_to_user(const char *data);

err_t tokenize(char *in, char **tokens);


// FLAG to not display info to user for send_info_to_user call
// have to be set before ever call, unset after
extern bool SILENT;

// FORMATTING
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define GREEN   "\033[32m"
#define BOLD    "\033[1m"
#define RESET   "\033[0m"


// SIGNALS
#define DB_ERR_GENERIC_FAIL     -1  // Command exec/proc FAIL signal
#define DB_ERR_OK               0   // Generic exec/proc PASS signal
#define DB_ERR_SUCCESS          1   // Command execution has been successful
#define DB_ERR_INVAILD_ARGS     2   // ARGS passed for command are incorrect
#define DB_ERR_NOMEM            3   // Unable to allocate memory

#define DB_ERR_KEY_NOTEXIST     4   // Key entry not present
#define DB_ERR_CMD_NOTEXIST     5   // Cmd sent does not exist

#define DB_ERR_EXIT             41  // EXIT signal for evnet loop

#endif