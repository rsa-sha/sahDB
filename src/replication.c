#include "replication.h"


// Signal mappers begin

master_repl_comm_request_sig map_string_to_enum(const char* signal) {
    if (strcmp(signal, REPL_JOIN) == 0) {
        return REPL_JOIN_SIG;
    } else if (strcmp(signal, REPL_INIT_JOIN) == 0) {
        return REPL_JOINED_SIG;
    } else if (strcmp(signal, REPL_JOINED) == 0) {
        return REPL_JOINED_SIG;
    } else if (strcmp(signal, REPL_LEAVE) == 0) {
        return REPL_LEAVE_SIG;
    } else if (strcmp(signal, REPL_LEFT) == 0) {
        return REPL_LEFT_SIG;
    }
    // Handle the case of an unknown signal
    fprintf(stderr, "Unknown signal: %s\n", signal);
    return -1; // Return an error code, or handle more appropriately
}

const char* map_enum_to_string(master_repl_comm_request_sig sig) {
    switch(sig) {
        case REPL_JOIN_SIG: return REPL_JOIN;
        case REPL_INIT_SIG: return REPL_INIT_JOIN;
        case REPL_JOINED_SIG: return REPL_JOINED;
        case REPL_LEAVE_SIG: return REPL_LEAVE;
        case REPL_LEFT_SIG: return REPL_LEFT;
        default: return "UNKNOWN_SIGNAL";
    }
}

err_t sig_send_req_to_mas(master_repl_comm_request_sig sig_send,
    master_repl_comm_request_sig sig_recv) {

    err_t res = DB_ERR_OK;
    char resp[MAX_RESP_LEN];

    // this will receive the stuff from Master
    char *buf = calloc(sizeof(char), MAX_CMD_LEN);
    int bytes_sent, bytes_recv;

    bytes_sent = send(
        host_config.master->sock, map_enum_to_string(sig_send),
        strlen(map_enum_to_string(sig_send)), 0);
    if (bytes_sent < 0) {
        sprintf(resp, "Unable to send data to Master");
        res = DB_ERR_GENERIC_FAIL;
        goto err_ret;
    }
    sprintf(resp, "Sent to master:%s [%d]", map_enum_to_string(sig_send), bytes_sent);
    send_info_to_user(resp);
    buf = calloc(sizeof(char), MAX_CMD_LEN);
    // sig from master
    bytes_recv = recv(host_config.master->sock, buf, MAX_CMD_LEN, 0);
    if (bytes_recv < 0) {
        sprintf(resp, "Unable to receive data from Master");
        res = DB_ERR_GENERIC_FAIL;
        goto err_ret;
    }
    sprintf(resp, "From master:%s [%d]", buf, bytes_recv);
    send_info_to_user(resp);
    // otherwise we sig OK
    send_info_to_user(buf);
    if (strcasecmp(buf, REPL_INIT_JOIN) == 0) {
        buf = calloc(sizeof(char), MAX_CMD_LEN);
        // master approved us for joining, now we send our details
        sprintf(buf, "DETAILS=%s:%d\0", host_config.ip, host_config.server_port);
        bytes_sent = send(host_config.master->sock, buf,
        strlen(buf), 0);
    }
    return res;

err_ret:
    // we close the sock and go back
    close(host_config.master->sock);
    host_config.master->is_conn = false;
    send_info_to_user(resp);
    return res;
}

// Signal mappers end


err_t master_init_handshake_with_repl(int repl_fd) {
    err_t res = DB_ERR_OK;
    char resp[MAX_RESP_LEN];
    char *buf = calloc(sizeof(char), MAX_CMD_LEN);
    int bytes_sent, bytes_recv;
    bytes_sent = send(repl_fd, REPL_INIT_JOIN, strlen(REPL_INIT_JOIN), 0);
    if (bytes_sent < 0) {
        sprintf(resp, "Unable to send data to potential replica");
        res = DB_ERR_GENERIC_FAIL;
        goto err_ret;
    }
    buf = calloc(sizeof(char), MAX_CMD_LEN);
    // sig from replica with it's info to be added to mem
    bytes_recv = recv(repl_fd, buf, MAX_CMD_LEN, 0);
    if (bytes_recv < 0) {
        sprintf(resp, "Unable to receive data from potential replica");
        res = DB_ERR_GENERIC_FAIL;
        goto err_ret;
    }
    // otherwise we sig OK
    sprintf(resp, "received %s from replica", buf);
    send_info_to_user(resp);
    buf = calloc(sizeof(char), MAX_CMD_LEN);
    return res;

err_ret:
    // we close the sock and go back
    close(repl_fd);
    host_config.master->is_conn = false;
    send_info_to_user(resp);
    return res;
}


// Streams file to socket
err_t send_save_file_to_repl(int sockfd, replicaNode repl, long start, long file_size) {
    err_t res = DB_ERR_OK;
    char resp[MAX_RESP_LEN];
    char buffer[MAX_RESP_LEN];
    FILE *fl = fopen(host_config.savefile, "rb");
    if (fl == NULL) {
        sprintf(resp, "Unable to open tmp savefile");
        res = DB_ERR_FILE_INACCESSIBLE;
        goto ret;
    }
    if (fseek(fl, start, SEEK_SET) != 0) {
        sprintf(resp, "Unable to move ptr to pos %ld in tmp savfile", start);
        res = DB_ERR_FILE_INACCESSIBLE;
        goto ret;
    }
    long bytes_read;
    while((bytes_read = fread(buffer, 1, MAX_RESP_LEN, fl)) > 0) {
        if (socket_send_data(sockfd, "%.*s", (long)bytes_read, buffer) != DB_ERR_OK) {
            res =  DB_ERR_GENERIC_FAIL;
            goto ret;
        }
    }
    // for(long i = start; i < file_size; i += MAX_RESP_LEN - 1) {
    //     strncpy(buffer, fl, MAX_RESP_LEN - 1);
    //     send_data(sockfd, "%s", buffer);
    // }
ret:
    if (fl != NULL) {
        fclose(fl);
    }
    send_info_to_user(resp);
    return res;
}

// Returns DB_ERR_OK if repl acknowledges handshake
static err_t replica_handshake_ack(int sock, replicaNode repl) {
    err_t res = DB_ERR_OK;
    char resp[MAX_RESP_LEN];
    // reading data on sock
    char *data = malloc(sizeof(char)*MAX_RESP_LEN);
    res = socket_read_data(sock, data, MAX_RESP_LEN-1);
    /*
    ACK format received from REPL:
    REPL_VERS=...;REPL_ID=...;OK
    */

    char *t1, *t2, *t3;
    t1 = strtok(data, ";");
    t2 = strtok(NULL, ";");
    t3 = strtok(NULL, ";");

    // vers tokens
    char *ver_key = strtok(t1, "=");
    char *ver_val = strtok(NULL, "=");

    // REPL UUID tokens
    char *uuid_key = strtok(t2, "=");
    char *uuid_val = strtok(NULL, "=");

    if (strcmp(t3, "OK") != 0) {
        res = DB_ERR_GENERIC_FAIL;
        // also to be in repl log
        send_info_to_user("Incomplete ack received from replica");
        goto ret;
    }
    // now we verify UUID
    if (strcmp(uuid_val, repl.uuid) != 0) {
        res = DB_ERR_GENERIC_FAIL;
        // also to be in repl log
        send_info_to_user("Replica UUID doesn't match");
        goto ret;    
    }
ret:
    free(data);
    return res;
}


/*
Master → Replica
-------------------------
HANDSHAKE (master → repl)
    REPL_VERS=0.0.1;
    MASTER_ID=<UUID>;
    DB_SIZE=<bytes>

HANDSHAKE ACK (repl → master)
    REPL_VERS=0.0.1;
    REPL_ID=<UUID>;
    OK

FULL DUMP STREAM (master → repl)
    DUMP_START
    <binary_data>   (exactly DB_SIZE bytes)
    DUMP_END

FINISH (master → repl)
    DONE

FINISH ACK (repl → master)
    OK

Connection closes
*/
static err_t share_data_with_replica(replicaNode repl, bool send_all) {
    err_t res = DB_ERR_OK;
    char resp[MAX_RESP_LEN];
    // initiate connection to the replica
    // int sockfd = connect_to_replica(repl.ip, repl.port);
    int sockfd = host_config.master->sock;
    if (sockfd < 0 || sockfd == NULL) {
        sprintf(resp, "unable to reach replica\n");
        goto ret;
    }
    long file_size = get_file_size(host_config.savefile);
    if (file_size < 1 ) {
        // nothing to send :(
        sprintf(resp, "Nothing in file");
        res = DB_ERR_GENERIC_FAIL;
        goto ret;
    }
    // HANDSHAKE from MASTER begin
    socket_send_data(sockfd, "REPL_VERS=%s;MASTER_ID=%s;DB_SIZE=%zu",
        REPL_VERS, host_config.uuid, file_size);
    // Handshake ACK from replica
    if (replica_handshake_ack(sockfd, repl)!=DB_ERR_OK) {
        sprintf(resp, "Replica %s did not ACK handshake from MASTER", repl.ip);
        res = DB_ERR_GENERIC_FAIL;
        goto ret;
    }
    // Send data stream
    if (send_all)
        res = send_save_file_to_repl(sockfd, repl, 0, file_size);
ret:
    send_info_to_user(resp);
    return res;
}


err_t replicatate_all() {
    // trigger internal save
    err_t res = DB_ERR_OK;
    char resp[MAX_RESP_LEN];
    if (host_config.replicas == NULL) {
        res = DB_ERR_GENERIC_FAIL;
        sprintf(resp, "There are no replicas to sync with.");
        goto ret;
    }
    char user_savefile[MAX_CMD_LEN];
    if (host_config.savefile != NULL)
        strcpy(user_savefile, host_config.savefile);
    host_config.savefile = TEMP_SAVE_FILE;
    // triggering save
    SILENT = 1;
    res = fetch_dataset_from_memory();
    SILENT = 0;
    switch (res) {
        case DB_ERR_FILE_INACCESSIBLE:
            sprintf(resp, "Unable to access temp savefile, somethings's wrong!!");
            break;
        default:
            // for O
            // sprintf(resp, "Generic error :(");
            break;
    }
    if (res)
        goto ret;

    // now we send stream to the replicas
    for (int i = 0; i< host_config.num_replicas; i++) {
        replicaNode repl = host_config.replicas[i];
        
        if ((repl.hostname == NULL && repl.ip == NULL) ||
            (repl.port < MIN_PORT || repl.port > MAX_PORT )) {
            // No point processsing as we can't share stuff
            continue;
        }
        if (repl.ip == NULL) {
            // we have to get it from DNS
            continue;
        }
        // now we connect to socket and send the repl stream
        share_data_with_replica(repl, true);
    }
ret:
    send_info_to_user(resp);
    return res;
} 



/////////////////////////////////////////////////////
/* Repl node info addition/removal related methods */
/////////////////////////////////////////////////////
err_t add_repl(/* some string from caller */) {
    err_t res = DB_ERR_OK;
    char resp[MAX_RESP_LEN];
    char *master_uuid;       // we'll get from string decoding
    char *replica_uuid;      // we'll get from string decoding
    char *replica_ip;        // we'll get from string decoding
    char *replica_port;      // we'll get from string decoding
    char *replica_hname;     // we'll get from string decoding
    // TODO: Will we need some kind of lock here (THINK!!)??
    // I guess'll need it when a replica is added/removed while replication (will work on this later)
    if (replica_port == NULL) {
        sprintf(resp, "The replica node has shared no port for comms; won't add");
        res = DB_ERR_INVAILD_ARGS;
        goto ret;
    }
    if (strcmp(host_config.uuid, master_uuid) != 0) {
        sprintf(resp, "The replica node [%s] has the wrong master UUID; won't add", master_uuid);
        res = DB_ERR_INVAILD_ARGS;
        goto ret;
    }
    if (replica_hname == NULL || replica_ip == NULL) {
        // TODO: we'll try to get these using DNS resolution
    }
    replicaNode *replica = malloc(sizeof(replicaNode));
    if (replica == NULL) {
        sprintf(resp, "Memory allocation failed for replica node");
        res = DB_ERR_NOMEM;
        goto ret;
    }
    replica->ip         = replica_ip;
    replica->hostname   = replica_hname;
    replica->port       = atoi(replica_port);
    replica->uuid       = replica_uuid;
    replica->next       = NULL;
    if (!host_config.num_replicas) {
        host_config.replicas = replica;
        host_config.num_replicas++;
        sprintf(resp, "Added replica info in Master's memory");
        goto ret;
    }
    replicaNode *temp = host_config.replicas;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = replica;
    sprintf(resp, "Added replica info in Master's memory");
    host_config.num_replicas++;
ret:
    // TODO: All of this should be in internal server log not the user log
    send_info_to_user(resp);
    return res;
}

err_t remove_repl(/* some string from caller */) {
    err_t res = DB_ERR_OK;
    char resp[MAX_RESP_LEN];
    char *replica_uuid;      // we'll get from string decoding
    // TODO: Will need lock here too (THINK!!)??
    // Same reason as in `add_repl`
    if (host_config.num_replicas == 0) {
        sprintf(resp, "There is no replica information with the master, skipping check");
        res = DB_ERR_GENERIC_FAIL;
        goto ret;
    }
    replicaNode *temp = host_config.replicas;
    replicaNode *removalNode = NULL;
    // if first node is the one
    if (strcmp(temp->uuid, replica_uuid) == 0) {
        // replicas points to the next
        host_config.replicas = temp->next;
        sprintf(resp, "Replica [%s] information removed from master", replica_uuid);
        removalNode = temp;
        goto free_repl;
    }
    // if a latter node is the one
    while (temp->next != NULL) {
        if (strcmp(temp->next->uuid, replica_uuid) == 0) {
            removalNode = temp->next;
            temp->next = temp->next->next;
            break;
        }
        temp = temp->next;
    }
    if (removalNode == NULL) {
        sprintf(resp, "No replica with ID [%s], in master's memory", replica_uuid);
        res = DB_ERR_GENERIC_FAIL;
        goto ret;
    }
free_repl:
    if (removalNode != NULL) {
        host_config.num_replicas--;
        free(removalNode->hostname);
        free(removalNode->ip);
        free(removalNode->uuid);
        if (removalNode->next)
            free(removalNode->next);
        free(removalNode);
        sprintf(resp, "Removed [%s], from master's memory", replica_uuid);
    }

ret:
    send_info_to_user(resp);
    return res;
}


err_t initialise_conn_with_master() {
    err_t res = DB_ERR_OK;
    char resp[MAX_RESP_LEN];
    int sockfd;
    struct sockaddr_in master_addr;
    struct hostent *host;
    host = gethostbyname(host_config.master->name);
    if (host == NULL) {
        sprintf(resp, "Unable to fecth IP of host %s", host_config.master->name);
        res = DB_ERR_GENERIC_FAIL;
        goto ret;
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        sprintf(resp, "Could not create socket");
        res = DB_ERR_GENERIC_FAIL;
        goto ret;
    }
    // sockaddr_in struct
    memset(&master_addr, 0, sizeof(struct sockaddr_in));
    master_addr.sin_family = AF_INET;
    master_addr.sin_port = htons(host_config.master->port);
    master_addr.sin_addr = *((struct in_addr*)host->h_addr_list[0]);

    // connecting to Master
    if (connect(sockfd, (struct sockaddr*)&master_addr, sizeof(master_addr)) < 0) {
        sprintf(resp, "Unable to connect to Master");
        res = DB_ERR_GENERIC_FAIL;
        close(sockfd);
        goto ret;
    }

    // send req to master for adding info of replica
    host_config.master->sock = sockfd;
    if (sig_send_req_to_mas(REPL_JOIN_SIG, REPL_JOINED_SIG) != 0) {
        // we did not receive the signal we were wishing for
    }

ret:
    send_info_to_user(resp);
    return res;
}