#pragma once
#include "security.h"
#include "config.h"
#include "db.h"

#ifdef DEBUG
#define MG_DEBUG_LVL "2"
#else
#define MG_DEBUG_LVL "1"
#endif

struct fantom_server_t {
	fantom_config_t *conf;
	fantom_db_t *db;
	fantom_nonce_manager_t *nonce_mgr;
}

void start_fantom_server(fantom_config_t *config, fantom_db_t *db);

