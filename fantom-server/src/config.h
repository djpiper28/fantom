#pragma once
#include "fantom_utils.h"
#include <stdio.h>

#define CONFIG_DEFAULT_BIND "http://127.0.0.1:8765"
#define CONFIG_FILE_NAME "fantom_config.json"
#define CONFIG_HELP "{\n" \
				"  \"db_file\": \"fantom.db\",\n" \
				"  \"bind_url\": \"" CONFIG_DEFAULT_BIND "\",\n" \
				"  \"max_log_age_days\": 5,\n" \
				"  \"jwt_expire\": 604800,\n" \
                                "  \"jwt_secret\": \"idowhd78atd76a5duisdYUTSRDASGDA89yid87as5d64asrtdua\"\n" \
				"}\n"

typedef struct fantom_config_t {
    char *db_file;
    char *bind_url; // where to bind the server to includes the port.
    char *jwt_secret;
    int max_log_age_days; // in days
    int jwt_expire;
} fantom_config_t;

void fantom_config_help();
fantom_status_t fantom_init_config(FILE *input, fantom_config_t *output);
void fantom_free_config(fantom_config_t *config);

