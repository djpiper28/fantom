#include <unistd.h>
#include "db.h"
#include "logger.h"

fantom_status_t init_db(fantom_db_t *fdb, char *db_file)
{
    int make_tables = 0;
    if (access(db_file, F_OK) != 0) {
        lprintf(LOG_WARNING, "The database file does not exist, making a new one\n");
        make_tables = 1;
    }

    // Open the database
    sqlite3 *db;
    int rc = sqlite3_open(db_file, &db);
    if (rc) {
        lprintf(LOG_ERROR, "Cannot open database %s\n", sqlite3_errmsg(db));
        return FANTOM_FAIL;
    }

    // Make tables
    if (make_tables) {
        char *err = NULL;
        int rc = sqlite3_exec(db, DB_CREATE_TABLES, NULL, NULL, &err);
        if (rc != SQLITE_OK) {
            lprintf(LOG_ERROR, "Cannot create tables %s\n", err);

            sqlite3_free(err);
            sqlite3_close(db);
            return FANTOM_FAIL;
        }

        lprintf(LOG_INFO, "A new database %s was made successfully\n", db_file);
    }

    // Copy to struct
    fdb->db = db;

    return FANTOM_SUCCESS;
}

void free_db(fantom_db_t *db)
{
    sqlite3_close(db);
}

