#ifndef REPLICATION_H
#define REPLICATION_H

#include "common.h"
#include "config.h"
#include "dbsave.h"
#include "hash.h"


// these are the communication headers between master and replicas not be used by anyone!!

#define REPL_JOIN           "REPL_JOIN_REQ"     // Initial msg from Replica -> Master 
#define REPL_INIT_JOIN      "REPL_INIT_JOIN"    // Master -> Replica after "REPL_JOIN_REQ", replica will send it's info
#define REPL_JOINED         "REPL_JOIN_OK"      // Master -> Replica after addition to replicas list on master
#define REPL_LEAVE          "REPL_LEAVE_REQ"    // Final msg header from Replica -> Master 
#define REPL_LEFT           "REPL_LEFT_OK"      // Final msg header's confirmation from Master -> Replica 


// these are the communication headers between master and user CLI !!
// User can't use NC <IP> <PORT> anymore, kind of
#define USER_JOIN           "USER_JOIN"         // for sahDB> prompt to user along with other things

/* Signals for Master-Replica Communication */
typedef enum master_repl_comm_request_sig {
    REPL_JOIN_SIG,          // Sent from Replica to Master for initial connection
    REPL_INIT_SIG,          // Sent from Master to Replica for receiving repl info for setup
    REPL_JOINED_SIG,        // Sent from Master to Replica after Master has added Repl info
    REPL_LEAVE_SIG,         // Sent from Replica to Master for connection termination
    REPL_LEFT_SIG,          // Sent from Master to Replica confirming connection termination
} master_repl_comm_request_sig;


extern server_config host_config;

#define TEMP_SAVE_FILE "data.safe.temp"
#define REPL_VERS "0.0.1"

extern HashTable* ht;

/* 
Method to init replica conn with master
Fetches master's info from struct based on conf
Sends Handshake req to Master to get added in network
    - If receives REPL_JOINED then we proceed
    - else nullify everything and quit
*/
err_t initialise_conn_with_master();

// For sending request sig from machine to master, checks the sig we wish to receive
err_t sig_send_req_to_mas(
    master_repl_comm_request_sig sig_send,
    master_repl_comm_request_sig sig_recv
);

/*
This is the initial handshake part on Master's end
Invoked when receive REPL_JOIN_SIG string from FD
*/
err_t master_init_handshake_with_repl(int repl_fd);

// String to signal enum mapper
master_repl_comm_request_sig map_string_to_enum(const char* signal);

// Signal enum to string to mapper
const char* map_enum_to_string(master_repl_comm_request_sig sig);


#endif