#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "lib/cjson/cJSON.h"
#include "network.h"
#include "data.h"

//LTV Communication Command
#define TSS_TO_LTV_RESET_COMMAND 4001

extern struct profile_context_t profile_context;

typedef struct server_context_t {
    SOCKET udp_socket;
    struct sockaddr_in ltv_addr;
    socklen_t ltv_addr_len;
    struct backend_data_t *backend;
} server_context_t; 

#endif // SERVER_H