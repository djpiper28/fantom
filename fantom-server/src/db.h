#pragma once
#include <sqlite3.h>
#include "fantom_utils.h"

#define DB_CREATE_TABLES ""

typedef struct fantom_db_t {
    sqlite3 *db
} fantom_db_t;

// Opens the db, if it is not present then it will make one and, create the tables.
fantom_status_t init_db(fantom_db_t *fdb, char *db_file);
void free_db(fantom_db_t *db);

