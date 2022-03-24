#include <unistd.h>
#include "db.h"
#include "logger.h"

char *DB_CREATE_TABLES =
    "create table users ("
    "  uid int primary key,"
    "  name varchar(64) not null,"
    "  salt varchar(256) not null,"
    "  password varchar(128) not null"
    ");"
    "create table admins ("
    "  uid int REFERENCES users(uid) not NULL"
    ");"
    "create table programs ("
    "  pid int primary key,"
    "  name varchar(256),"
    "  created_time int"
    ");"
    "create table log_entry ("
    "  pid int REFERENCES programs(pid) not null,"
    "  start_time int not null,"
    "  end_time int not null,"
    "  memory_usage int not null,"
    "  cpu_time int not null,"
    "  max_threads int not NULL"
    ");"
    "create table user_prog_prefs ("
    "  uid int REFERENCES users(uid) not null,"
    "  pid int REFERENCES programs(pid) not null,"
    "  colour int,"
    "  favourite bool not null DEFAULT false,"
    "  hidden bool not null DEFAULT false,"
    "  name varchar(256),"
    "  check ((favourite or hidden) and not (favourite and hidden))"
    ");"
    "insert into users (uid, name, salt, password) "
    "values (1, 'admin', "
    "'" DEFAULT_PASSWORD_SALT "', "
    "'" DEFAULT_PASSWORD_HASH "');"
    "insert into admins (uid) values (1);";

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
        lprintf(LOG_WARNING, "A default user called '"
                ANSI_YELLOW "admin" ANSI_RESET "' with password '"
                ANSI_YELLOW DEFAULT_PASSWORD ANSI_RESET
                "' was made, please login and "
                ANSI_RED "change the password" ANSI_RESET "\n");
    }

    // Copy to struct
    fdb->db = db;

    return FANTOM_SUCCESS;
}

void free_db(fantom_db_t *db)
{
    sqlite3_close(db->db);
}

