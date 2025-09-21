#ifndef COMMAND_H
#define COMMAND_H

#include "common.h"

/********* DB Page *********/ 
#define MAX_KEY_SIZE 32
#define MAX_VALUE_SIZE 128
#define MAX_RECORDS 25
// Amounts to 4k (4096) ~ (32+128)*25 = [4000]
typedef struct KV{
    char key[MAX_KEY_SIZE];
    char val[MAX_VALUE_SIZE];
} KV;
typedef struct Page{
    KV records[MAX_RECORDS];
    int num_records;            // num of stored records in page
} Page;

/*********  *********/ 
/*********  *********/ 
/********* Tokenization struct *********/ 
typedef err_t (*func_ptr)(int argc, char **cmd_arr);
typedef struct CommandNode{
    char *name;                     // COMMAND Names -> "set", "get"
    struct CommandNode *subcmds;    // command subcommands
    bool isCmd;                     // If true then we call func_ptr
    int depth;                      // for tree traversal
    func_ptr func;
} commandNode;

#define N_COMMANDS 4


/********** HELP COMMAND **********/
err_t help();



/********** SET COMMAND **********/
err_t set_key_val(int argc, char **cmd_arr);


/********** GET COMMAND **********/
err_t get_val_from_key(int argc, char **cmd_arr);

#endif