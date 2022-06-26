#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "./security.h"
#include "./db.h"
#include "./logger.h"

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
    "values (1, '"DEFAULT_USER"', "
    "'" DEFAULT_PASSWORD_SALT "', "
    "'" DEFAULT_PASSWORD_HASH "');"
    "insert into admins (uid) values (1);";

#define GET_USER_BY_NAME_SQL "select uid, name, salt, password from users where name = ?;"
#define GET_USER_BY_UID_SQL "select name, salt, password from users where uid = ?;"

fantom_status_t init_db(fantom_db_t *fdb, char *db_file)
{
    memset(fdb, 0, sizeof(*fdb));
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
    char *err = NULL;
    if (make_tables) {
        int rc = sqlite3_exec(db, DB_CREATE_TABLES, NULL, NULL, &err);
        if (rc != SQLITE_OK) {
            lprintf(LOG_ERROR, "Cannot create tables %s\n", err);

            sqlite3_free(err);
            sqlite3_close(db);
            return FANTOM_FAIL;
        }

        lprintf(LOG_INFO, "A new database %s was made successfully\n", db_file);
        lprintf(LOG_WARNING, "A default user called '"
                ANSI_YELLOW DEFAULT_USER ANSI_RESET "' with password '"
                ANSI_YELLOW DEFAULT_PASSWORD ANSI_RESET
                "' was made, please login and "
                ANSI_RED "change the password" ANSI_RESET ".\n"
                ANSI_RED "You cannot use the account until the password is changed.\n" ANSI_RESET);
    }

    // Copy to struct
    fdb->db = db;

    // Init prepared statements
    // get user
    rc = sqlite3_prepare_v3(fdb->db,
                            GET_USER_BY_UID_SQL,
                            strlen(GET_USER_BY_UID_SQL),
                            0,
                            &fdb->get_user_stmt,
                            NULL);

    if (rc != SQLITE_OK) {
        lprintf(LOG_ERROR, "Cannot init get user stmt %s\n", err);

        sqlite3_free(err);
        return FANTOM_FAIL;
    }

    // login
    rc = sqlite3_prepare_v3(fdb->db,
                            GET_USER_BY_NAME_SQL,
                            strlen(GET_USER_BY_NAME_SQL),
                            0,
                            &fdb->login_stmt,
                            NULL);

    if (rc != SQLITE_OK) {
        lprintf(LOG_ERROR, "Cannot init login stmt %s\n", err);

        sqlite3_free(err);
        return FANTOM_FAIL;
    }

    return FANTOM_SUCCESS;
}

void free_db(fantom_db_t *db)
{
    if (db->db != NULL) {
        sqlite3_close(db->db);
    }

    if (db->get_user_stmt != NULL) {
        sqlite3_finalize(db->get_user_stmt);
    }

    if (db->login_stmt != NULL) {
        sqlite3_finalize(db->login_stmt);
    }
}

// Model

void free_user(fantom_user_t *user)
{
    if (user->name != NULL) {
        free(user->name);
    }
    user->name = NULL;
    user->uid = -1;
}

void free_users(fantom_users_t *users)
{
    if (users->users != NULL) {
        free(users->users);
    }
    users->users = NULL;
    users->length = 0;
}

static int is_default_password(char *password, char *salt)
{
    return strcmp(password, DEFAULT_PASSWORD_HASH) == 0 && strcmp(salt, DEFAULT_PASSWORD_SALT) == 0;
}

fantom_status_t db_get_user(fantom_db_t *fdb, int uid, fantom_user_t *ret)
{
    sqlite3_reset(fdb->get_user_stmt);
    ret->uid = -1;
    char *err = NULL;
    int rc = sqlite3_bind_int(fdb->get_user_stmt,
                              1,
                              uid);
    if (rc != SQLITE_OK) {
        lprintf(LOG_ERROR, "Cannot get user (uid %d) %s\n", uid, err);

        sqlite3_free(err);
        return FANTOM_FAIL;
    }


    char *name = NULL;
    char *salt = NULL;
    char *password = NULL;
    while (SQLITE_ROW == sqlite3_step(fdb->get_user_stmt)) {
        char *argv = (char *) sqlite3_column_text(fdb->get_user_stmt, 0);
        char *tmp = malloc(strlen(argv) + 1);
        if (tmp == NULL) {
            lprintf(LOG_ERROR, "Malloc error\n");
            return 0;
        }

        strcpy(tmp, argv);
        name = tmp;

        argv = (char *) sqlite3_column_text(fdb->get_user_stmt, 1);
        tmp = malloc(strlen(argv) + 1);
        if (tmp == NULL) {
            lprintf(LOG_ERROR, "Malloc error\n");
            free(name);
            name = NULL;
            return 0;
        }

        strcpy(tmp, argv);
        salt = tmp;

        argv = (char *) sqlite3_column_text(fdb->get_user_stmt, 2);
        tmp = malloc(strlen(argv) + 1);
        if (tmp == NULL) {
            lprintf(LOG_ERROR, "Malloc error\n");
            free(name);
            free(salt);
            name = NULL;
            salt = NULL;
            return 0;
        }

        strcpy(tmp, argv);
        password = tmp;
    }

    if (name == NULL) {
        lprintf(LOG_ERROR, "Cannot find user (uid %d)\n", uid);
        return FANTOM_FAIL;
    }
    ret->uid = uid;
    ret->name = name;

    if (is_default_password(password, salt)) {
        ret->status = FANTOM_USER_PASSWORD_NEEDS_CHANGE;
    } else if (strcmp(salt, "") == 0 || strcmp(password, "") == 0) {
        ret->status = FANTOM_USER_ACCOUNT_LOCKED;
    } else {
        ret->status = FANTOM_USER_VALID;
    }

    if (salt != NULL) {
        free(salt);
    }

    if (password != NULL) {
        free(password);
    }

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
            free(tmp);
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
        default:
            lprintf(LOG_WARNING, "Unrecognised column %s\n", col_names[i]);
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

typedef struct fantom_login_t {
    fantom_user_t *ret;
    char *password;
} fantom_login_t;

fantom_status_t db_login(fantom_db_t *fdb, char *name, char *password_in, fantom_user_t *ret)
{
    sqlite3_reset(fdb->login_stmt);
    ret->uid = -1;
    char *err = NULL;
    int rc = sqlite3_bind_text(fdb->login_stmt,
                               1,
                               name,
                               strlen(name),
                               SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        lprintf(LOG_ERROR, "Cannot bind login stmt %s\n", err);

        sqlite3_free(err);
        return FANTOM_FAIL;
    }

    char *salt = NULL;
    char *password = NULL;
    while (SQLITE_ROW == sqlite3_step(fdb->login_stmt)) {
        ret->uid = sqlite3_column_int(fdb->login_stmt, 0);

        char *argv = (char *) sqlite3_column_text(fdb->login_stmt, 1);
        char *tmp = malloc(strlen(argv) + 1);
        if (tmp == NULL) {
            lprintf(LOG_ERROR, "Malloc error\n");
            return 0;
        }

        strcpy(tmp, argv);
        ret->name = tmp;

        argv = (char *) sqlite3_column_text(fdb->login_stmt, 2);
        tmp = malloc(strlen(argv) + 1);
        if (tmp == NULL) {
            lprintf(LOG_ERROR, "Malloc error\n");
            free(ret->name);
            ret->name = NULL;
            return 0;
        }

        strcpy(tmp, argv);
        salt = tmp;

        argv = (char *) sqlite3_column_text(fdb->login_stmt, 3);
        tmp = malloc(strlen(argv) + 1);
        if (tmp == NULL) {
            lprintf(LOG_ERROR, "Malloc error\n");
            free(ret->name);
            free(salt);
            ret->name = NULL;
            salt = NULL;
            return 0;
        }

        strcpy(tmp, argv);
        password = tmp;
    }

    if (salt != NULL && password != NULL) {
        // Check password
        char *hpwd = hash_password(password_in, salt);
        if (hpwd != NULL) {
            if (strcmp(password, hpwd) != 0) {
                free_user(ret);
                lprintf(LOG_WARNING, "Invalid password\n");
            } else {
                if (is_default_password(password, salt)) {
                    ret->status = FANTOM_USER_PASSWORD_NEEDS_CHANGE;
                } else if (strcmp(salt, "") == 0 || strcmp(password, "") == 0) {
                    ret->status = FANTOM_USER_ACCOUNT_LOCKED;
                } else {
                    ret->status = FANTOM_USER_VALID;
                }
            }

            free(hpwd);
        } else {
            lprintf(LOG_ERROR, "Malloc error\n");
            free_user(ret);
        }
    }

    free(salt);
    free(password);

    if (ret->uid == -1) {
        lprintf(LOG_WARNING, "Cannot login user (name %s)\n", name);
        return FANTOM_FAIL;
    }

    return FANTOM_SUCCESS;
}

