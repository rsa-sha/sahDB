#ifndef CLI_H
#define CLI_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include "common.h"
#include "config.h"

// STATES OF Connection
#define CLI_NO_CONN     (1ULL << 0)     // Initial state of CLI Conn
#define CLI_CONN_INIT   (1ULL << 1)     // CLI connected to server successfully
#define CLI_COMM_PROC   (1ULL << 2)     // Have sent a cmd to server, waiting for resp

// Handles options passed by user
typedef struct cli_s {
    bool            local;              // Is the server on same machine
    char            *ip;                // IP addr of the server
    int             port;               // Port on which server is running
    char            resp[MAX_RESP_LEN]; // For internal func resp str only
    int             sockfd;             // Socket FD
    
    struct sockaddr_in     server;      // Socket to Master
} cli_s;

#define HANDSHAKE_INIT      "USER_JOIN"
#define HANDSHAKE_INIT_LEN  9

extern AppConfig app_config;
/*
init_cli_config takes args passed from terminal
and builds up the struct for CLI options
*/
err_t init_cli_config(int argc, char **argv);



/*********************************
***** DOCUMENTATION FOR SELF *****
*********************************/
/*
CLI interaction with server
0. Check and initiate port, host vars in-mem 
    happen in main() from user_args => will move to simpler check_and_populate_args()

1. Initiate connection with server "USER_JOIN"
    Happens in handshake_with_server() [Assumes server is verified in Step 0] -
    - Create socket for comms
    - Init connection with server
    - Send "USER_JOIN" to server to tell socket is being used by USER

2. Processing user requests
    Questions: 
    - Does user really need SIG numbers of commands? 
        [Think no, EXIT and SHUTDOWN are the only two meaningful ones]
        [which can be handled with resp strings of Server]
    - What if the user enters escape sequences or whitespaces
        [Let's let the Server process them for now, and see what happens]
    Major hiccup:
    - Reading user input?
        [When USER_JOIN is sent the server responses with ]
*/

#endif
