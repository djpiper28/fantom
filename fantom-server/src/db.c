#include <unistd.h>
#include <stdlib.h>
#include <string.h>
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

// Model

static int is_default_password(char *password, char *salt)
{
    return strcmp(password, DEFAULT_PASSWORD_HASH) == 0 && strcmp(salt, DEFAULT_PASSWORD_SALT) == 0;
}

static int db_get_user_callback(void *ret_in, int argc, char **argv, char ** col_names)
{
    if (argc == 0) {
        lprintf(LOG_ERROR, "No cols in row\n");
        return 0;
    }

    fantom_user_t *ret = (fantom_user_t *) ret_in;
    char *salt = NULL;
    char *password = NULL;

    for (int i = 0; i < argc; i++) {
        char *tmp = malloc(strlen(argv[i]) + 1);
        switch (i) {
        case 0:
            if (tmp == NULL) {
                lprintf(LOG_ERROR, "Malloc error\n");
                return 0;
            }

            strcpy(tmp, argv[i]);
            ret->name = tmp;
            break;
        case 1:
            if (tmp == NULL) {
                lprintf(LOG_ERROR, "Malloc error\n");
                free(ret->name);
                ret->name = NULL;
                return 0;
            }

            strcpy(tmp, argv[i]);
            salt = tmp;
            break;
        case 2:
            if (tmp == NULL) {
                lprintf(LOG_ERROR, "Malloc error\n");
                free(ret->name);
                free(salt);
                ret->name = NULL;
                salt = NULL;
                return 0;
            }

            strcpy(tmp, argv[i]);
            password = tmp;
            break;
        }
    }

    if (is_default_password(password, salt)) {
        ret->status = FANTOM_USER_PASSWORD_NEEDS_CHANGE;
    } else if (strcmp(salt, "") == 0 || strcmp(password, "") == 0) {
        ret->status = FANTOM_USER_ACCOUNT_LOCKED;
    } else {
        ret->status = FANTOM_USER_VALID;
    }

    free(salt);
    free(password);

    ret->uid = 0;
    return 0;
}

fantom_status_t db_get_user(fantom_db_t *fdb, int uid, fantom_user_t *ret)
{
    ret->uid = -1;
    char *err = NULL;
    char sql[256];
    snprintf(sql, sizeof(sql), "select name, salt, password from users where uid=%d;", uid);

    int rc = sqlite3_exec(fdb->db, sql,
                          &db_get_user_callback,
                          (void *) ret,
                          &err);
    if (rc != SQLITE_OK) {
        lprintf(LOG_ERROR, "Cannot get user (uid %d) %s\n", uid, err);

        sqlite3_free(err);
        return FANTOM_FAIL;
    }

    if (ret->uid == -1) {
        lprintf(LOG_ERROR, "Cannot find user (uid %d)\n", uid);
        return FANTOM_FAIL;
    }
    ret->uid = uid;

    return FANTOM_SUCCESS;
}

static int db_get_users_callback(void *ret_in, int argc, char **argv, char ** col_names)
{
    if (argc == 0) {
        lprintf(LOG_ERROR, "No cols in row\n");
        return 0;
    }

    fantom_users_t *ret_arr = (fantom_users_t *) ret_in;
    if (ret_arr->users == NULL) {
        if (ret_arr->length == 0) {
            ret_arr->users = malloc(sizeof * ret_arr->users);
            if (ret_arr->users == NULL) {
                lprintf(LOG_ERROR, "Malloc error\n");
                return 1;
            }
        } else {
            lprintf(LOG_ERROR, "Null array\n");
            return 1;
        }
    } else {
        ret_arr->users = realloc(ret_arr->users, ret_arr->length + sizeof(*ret_arr->users));
        if (ret_arr->users == NULL) {
            lprintf(LOG_ERROR, "Realloc error\n");
            return 1;
        }
    }

    ret_arr->length++;
    fantom_user_t *ret = &ret_arr->users[ret_arr->length - 1];

    char *salt = NULL;
    char *password = NULL;

    for (int i = 0; i < argc; i++) {
        char *tmp = malloc(strlen(argv[i]) + 1);
        switch (i) {
        case 0:
            ret->uid = atoi(argv[i]);
            break;
        case 1:
            if (tmp == NULL) {
                lprintf(LOG_ERROR, "Malloc error\n");
                return 0;
            }

            strcpy(tmp, argv[i]);
            ret->name = tmp;
            break;
        case 2:
            if (tmp == NULL) {
                lprintf(LOG_ERROR, "Malloc error\n");
                free(ret->name);
                ret->name = NULL;
                return 0;
            }

            strcpy(tmp, argv[i]);
            salt = tmp;
            break;
        case 3:
            if (tmp == NULL) {
                lprintf(LOG_ERROR, "Malloc error\n");
                free(ret->name);
                free(salt);
                ret->name = NULL;
                salt = NULL;
                return 0;
            }

            strcpy(tmp, argv[i]);
            password = tmp;
            break;
        }
    }

    if (is_default_password(password, salt)) {
        ret->status = FANTOM_USER_PASSWORD_NEEDS_CHANGE;
    } else if (strcmp(salt, "") == 0 || strcmp(password, "") == 0) {
        ret->status = FANTOM_USER_ACCOUNT_LOCKED;
    } else {
        ret->status = FANTOM_USER_VALID;
    }

    free(salt);
    free(password);
    return 0;
}

fantom_status_t db_get_all_users(fantom_db_t *fdb, fantom_users_t *ret)
{
    ret->users = NULL;
    ret->length = 0;

    char *err = NULL;
    int rc = sqlite3_exec(fdb->db,
                          "select uid, name, salt, password from users;",
                          &db_get_users_callback,
                          (void *) ret,
                          &err);
    if (rc != SQLITE_OK) {
        lprintf(LOG_ERROR, "Cannot get users %s\n", err);

        sqlite3_free(err);
        return FANTOM_FAIL;
    }

    return FANTOM_SUCCESS;
}

