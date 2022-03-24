#pragma once
#include "config.h"
#include "db.h"

#ifdef DEBUG
#define MG_DEBUG_LVL "2"
#else
#define MG_DEBUG_LVL "1"
#endif

void start_fantom_server(fantom_config_t *config, fantom_db_t *db);

