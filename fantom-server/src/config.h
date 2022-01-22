#pragma once
#include "fantom_str.h"
#include "fantom_utils.h"

typedef struct fantom_config_t {
  fantom_str_t bind_url;
  fantom_str_t jwt_secret;
  fantom_str_t db_file;
  int max_nonces;
  int max_connections;
} fantom_config_t;

fantom_status_t get_default_fantom_config(fantom_config_t *);

