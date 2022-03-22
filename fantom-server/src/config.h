#pragma once
#include "fantom_utils.h"
#include <stdio.h>

#define CONFIG_FILE_NAME "fantom_config.json"
#define CONFIG_HELP "{\n" \
				"  \"db_file\": \"fantom.db\",\n" \
				"  \"bind_url\": \"http://127.0.0.1:8765\",\n" \
				"  \"max_log_age_days\": 5\n" \
				"}\n"

typedef struct fantom_config_t {
    char *db_file;
    char *bind_url; // where to bind the server to includes the port.
    int max_log_age_days; // in days
} fantom_config_t;

void fantom_config_help();
fantom_status_t fantom_init_config(FILE *input, fantom_config_t *output);
fantom_status_t fantom_free_config(fantom_config_t *config);

