#pragma once
#include "security.h"
#include "config.h"
#include "db.h"
#include "mongoose.h"

#ifdef DEBUG
#define MG_DEBUG_LVL "2"
#else
#define MG_DEBUG_LVL "1"
#endif

#define MIN(a, b) (a < b ? a : b)

typedef struct fantom_server_t {
    fantom_config_t *conf;
    fantom_db_t *db;
    fantom_nonce_manager_t *nonce_mgr;
} fantom_server_t;

#ifdef TEST
extern int server_running_override;
#endif

void start_fantom_server(fantom_config_t *config, fantom_db_t *db);
void send_500_error(struct mg_connection *c);
void send_403_error(struct mg_connection *c);
void send_400_error(struct mg_connection *c);
fantom_status_t check_nonce(struct mg_connection *c, fantom_server_t s, struct mg_http_message *hm);

