#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define err_t int
// IO_Related Methods
#define MAX_CMD_LEN 512
#define MAX_CMD_PARAMS 16
#define MAX_RESP_LEN 81192
err_t get_user_input(char* cmd);

void send_info_to_user(char *data);

err_t tokenize(char *in, char **tokens);


// FORMATTING
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define BOLD    "\033[1m"
#define RESET   "\033[0m"


// SIGNALS
#define SIG_EXIT 41
#endif