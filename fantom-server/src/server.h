#pragma once
#include "config.h"

#ifdef DEBUG
#define MG_DEBUG "2"
#else
#define MG_DEBUG "1"
#endif

void start_fantom_server(fantom_config_t *config);

