#include <string.h>
#include "mongoose.h"
#include "logger.h"
#include "server.h"

void start_fantom_server(fantom_config_t config)
{
	  lprintf(LOG_INFO, "Starting server on %s with the database as %s.\n", config.bind_url, config.db_file);
}

