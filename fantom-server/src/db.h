#pragma once
#include <sqlite3.h>
#include "fantom_utils.h"

#define DEFAULT_PASSWORD_SALT ""
#define DEFAULT_PASSWORD_HASH "2ab7f298204c8588cd304395d08d4baa855b1baedc5db54031b0901533f9cc77d6cb33823587f96984ea57f069a4c15952c5043e0ab7aa53669f2b81ff75268f"
#define DEFAULT_PASSWORD "@dM1n"

typedef struct fantom_db_t {
    sqlite3 *db;
} fantom_db_t;

// Opens the db, if it is not present then it will make one and, create the tables.
fantom_status_t init_db(fantom_db_t *fdb, char *db_file);
void free_db(fantom_db_t *db);

