#ifndef COMMAND_H
#define COMMAND_H

#include "common.h"
#include "config.h"

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


// COMMAND CONTEXT STRUCTURE
typedef struct cmd_ctx {
    conn_t  *conn;              // FOR I/O over network
    char    *resp;              // Response to be sent over to the client
    err_t   status;             // Return code of last processed request [used for conn state change/handshake]
} cmd_ctx;

/*********  *********/ 
/*********  *********/ 
/********* Tokenization struct *********/ 
typedef err_t (*func_ptr)(int argc, char **cmd_arr, cmd_ctx *conn);
typedef struct CommandNode{
    char *name;                     // COMMAND Names -> "set", "get"
    struct CommandNode *subcmds;    // command subcommands
    bool isCmd;                     // If true then we call func_ptr
    int depth;                      // for tree traversal
    func_ptr func;
    const char *usage;
} commandNode;

#define N_COMMANDS 9
/*
 * HELP     -> Display generic and specific help for stuff
 * SET      -> Modify/Add value for a given key
 * EXPIRE   -> Modify/Add expiration time for a given key
 * GET      -> Fetch value for a given key
 * EXISTS   -> Check if a key exists in DB
 * DELETE   -> Remove key and corresponding entry from DB
 * SAVE     -> SAVE the in-memory data to a file on disk
 * EXIT     -> Exit out of the User|Replica connection
 * SHUTDOWN -> SHUTDOWN the DB server
*/

#define MAX(a,b) ((a) > (b) ? (a) : (b))
/********** HELP COMMAND **********/
err_t help(cmd_ctx *conn);

/********** SET COMMAND **********/
err_t set_key_val(int argc, char **cmd_arr, cmd_ctx *conn);

/********** GET COMMAND **********/
err_t get_val_from_key(int argc, char **cmd_arr, cmd_ctx *conn);

/******** EXISTS COMMAND *********/
err_t key_exists(int argc, char **cmd_arr, cmd_ctx *conn);

/********* DELETE COMMAND ********/
err_t delete_key_val(int argc, char **cmd_arr, cmd_ctx *conn);

/********** EXPIRE COMMAND **********/
err_t expire_key_val(int argc, char **cmd_arr, cmd_ctx *conn);

/********** FILL CMD_CTX FROM STACK PARAMS **********/
void fill_cmd_ctx(cmd_ctx *ctx, err_t res, const char* resp);

/********** CMD CTX WRITE HELPER **********/
err_t cmd_ctx_write(cmd_ctx *ctx, const char *data);
#endif
