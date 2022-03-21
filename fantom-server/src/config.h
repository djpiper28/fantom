#pragma once
#include "fantom_utils.h"
#include <stdio.h>

typedef struct fantom_config_t {
    char *db_name;
    char *db_username;
    char *db_password;
    char *db_url; // includes the port.
    char *bind_url; // where to bind the server to includes the port.
    int max_log_age_days; // in days
} fantom_config_t;

void fantom_config_help();
fantom_status_t init_config(FILE *input, fantom_config_t *output);
fantom_status_t free_config(fantom_config_t *config);

