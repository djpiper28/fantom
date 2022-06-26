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
    
    "create index users_uid on users(uid);"
    "create index users_name on users(name);"
    
    "create table admins ("
    "  uid int REFERENCES users(uid) not NULL"
    ");"
    
    "create index admins_uid on admins(uid);"
    
    "create table programs ("
    "  pid int primary key,"
    "  name varchar(256),"
    "  created_time int"
    ");"
    
    "create index programs_pid on programs(pid);"
    "create index programs_name on programs(name);"
    
    "create table log_prog_entry ("
    "  pid int REFERENCES programs(pid) not null,"
    "  time int not null,"
    "  memory_usage int not null,"
    "  cpu_usage int not null,"
    "  cpu_count int not null"
    ");"
    
    "create index log_prog_entry_pid on log_prog_entry(pid);"
    "create index log_prog_entry_time on log_prog_entry(time);"
    
    "create table log_entry ("
    "  time int not null,"
    "  memory_usage int not null,"
    "  cpu_usage int not null,"
    "  cpu_count int not null"
    ");"
    
    "create index log_entry_time on log_entry(time);"
    
    "create table user_prog_prefs ("
    "  uid int REFERENCES users(uid) not null,"
    "  pid int REFERENCES programs(pid) not null,"
    "  colour int,"
    "  favourite bool not null DEFAULT false,"
    "  hidden bool not null DEFAULT false,"
    "  name varchar(256),"
    "  check ((favourite or hidden) and not (favourite and hidden))"
    ");"

    "create index user_prog_prefs_uid on user_prog_prefs(uid);"
    "create index user_prog_prefs_pid on user_prog_prefs(pid);"

    "insert into users (uid, name, salt, password) "
    "values (1, '"DEFAULT_USER"', "
    "'" DEFAULT_PASSWORD_SALT "', "
    "'" DEFAULT_PASSWORD_HASH "');"
    "insert into admins (uid) values (1);";

#define GET_USER_BY_NAME_SQL "select uid, name, salt, password from users where name = ?;"
#define GET_USER_BY_UID_SQL "select name, salt, password from users where uid = ?;"
#define UPDATE_USER_PASSWORD "update users set password = ?, salt = ? where uid = ?;"

fantom_status_t init_db(fantom_db_t *fdb, char *db_file)
{
    memset(fdb, 0, sizeof(*fdb));
    int make_tables = 0;
    if (access(db_file, F_OK) != 0) {
        lprintf(LOG_WARNING, "The database file does not exist, making a new one\n");
        make_tables = 1;
    }

    // Open the database
    int rc = sqlite3_open(db_file, &fdb->db);
    if (rc) {
        lprintf(LOG_ERROR, "Cannot open database %s\n", sqlite3_errmsg(fdb->db));
        return FANTOM_FAIL;
    }

    // Make tables
    char *err = NULL;
    if (make_tables) {
        int rc = sqlite3_exec(fdb->db, DB_CREATE_TABLES, NULL, NULL, &err);
        if (rc != SQLITE_OK) {
            lprintf(LOG_ERROR, "Cannot create tables %s\n", err);

            sqlite3_free(err);
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

    // Init prepared statements
    // get user
    rc = sqlite3_prepare_v3(fdb->db,
                            GET_USER_BY_UID_SQL,
                            strlen(GET_USER_BY_UID_SQL),
                            0,
                            &fdb->get_user_stmt,
                            NULL);

    if (rc != SQLITE_OK) {
        lprintf(LOG_ERROR, "Cannot init get user stmt %s\n", sqlite3_errmsg(fdb->db));
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
        lprintf(LOG_ERROR, "Cannot init login stmt %s\n", sqlite3_errmsg(fdb->db));
        return FANTOM_FAIL;
    }

    // change password
    rc = sqlite3_prepare_v3(fdb->db,
                            UPDATE_USER_PASSWORD,
                            strlen(UPDATE_USER_PASSWORD),
                            0,
                            &fdb->change_password_stmt,
                            NULL);

    if (rc != SQLITE_OK) {
        lprintf(LOG_ERROR, "Cannot init change password stmt %s\n", sqlite3_errmsg(fdb->db));
        return FANTOM_FAIL;
    }

    return FANTOM_SUCCESS;
}

void free_db(fantom_db_t *db)
{
    if (db->get_user_stmt != NULL) {
        sqlite3_finalize(db->get_user_stmt);
    }

    if (db->login_stmt != NULL) {
        sqlite3_finalize(db->login_stmt);
    }

    if (db->change_password_stmt != NULL) {
        sqlite3_finalize(db->change_password_stmt);
    }

    if (db->db != NULL) {
        sqlite3_close(db->db);
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
    ret->uid = -1;
    int rc = sqlite3_bind_int(fdb->get_user_stmt,
                              1,
                              uid);
    if (rc != SQLITE_OK) {
        lprintf(LOG_ERROR, "Cannot get user (uid %d) %s\n", uid, sqlite3_errmsg(fdb->db));
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
            sqlite3_reset(fdb->get_user_stmt);
            return FANTOM_FAIL;
        }

        strcpy(tmp, argv);
        name = tmp;

        argv = (char *) sqlite3_column_text(fdb->get_user_stmt, 1);
        tmp = malloc(strlen(argv) + 1);
        if (tmp == NULL) {
            lprintf(LOG_ERROR, "Malloc error\n");
            free(name);
            sqlite3_reset(fdb->get_user_stmt);
            return FANTOM_FAIL;
        }

        strcpy(tmp, argv);
        salt = tmp;

        argv = (char *) sqlite3_column_text(fdb->get_user_stmt, 2);
        tmp = malloc(strlen(argv) + 1);
        if (tmp == NULL) {
            lprintf(LOG_ERROR, "Malloc error\n");
            free(name);
            free(salt);
            sqlite3_reset(fdb->get_user_stmt);
            return FANTOM_FAIL;
        }

        strcpy(tmp, argv);
        password = tmp;
    }

    if (name == NULL) {
        lprintf(LOG_ERROR, "Cannot find user (uid %d)\n", uid);
        sqlite3_reset(fdb->get_user_stmt);
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

    free(salt);
    free(password);
    sqlite3_reset(fdb->get_user_stmt);

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
    ret->uid = -1;
    int rc = sqlite3_bind_text(fdb->login_stmt,
                               1,
                               name,
                               strlen(name),
                               SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        lprintf(LOG_ERROR, "Cannot bind login stmt %s\n", sqlite3_errmsg(fdb->db));
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
            sqlite3_reset(fdb->login_stmt);
            return FANTOM_FAIL;
        }

        strcpy(tmp, argv);
        ret->name = tmp;

        argv = (char *) sqlite3_column_text(fdb->login_stmt, 2);
        tmp = malloc(strlen(argv) + 1);
        if (tmp == NULL) {
            lprintf(LOG_ERROR, "Malloc error\n");
            free(ret->name);
            sqlite3_reset(fdb->login_stmt);
            return FANTOM_FAIL;
        }

        strcpy(tmp, argv);
        salt = tmp;

        argv = (char *) sqlite3_column_text(fdb->login_stmt, 3);
        tmp = malloc(strlen(argv) + 1);
        if (tmp == NULL) {
            lprintf(LOG_ERROR, "Malloc error\n");
            free(ret->name);
            free(salt);
            salt = NULL;
            sqlite3_reset(fdb->login_stmt);
            return FANTOM_FAIL;
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
    sqlite3_reset(fdb->login_stmt);

    if (ret->uid == -1) {
        lprintf(LOG_WARNING, "Cannot login user (name %s)\n", name);
        return FANTOM_FAIL;
    }

    return FANTOM_SUCCESS;
}

fantom_status_t db_change_password(fantom_db_t *fdb, char *uid, char *new_password)
{
    if (strlen(new_password) < MINIMUM_PASSWORD_LENGTH) {
        lprintf(LOG_ERROR,
                "User %d's password was attempted to be set to a length less than the minimum %d\n",
                uid,
                MINIMUM_PASSWORD_LENGTH);
        return FANTOM_FAIL;
    }

    char *salt = get_salt();
    if (salt == NULL) {
        lprintf(LOG_ERROR, "Cannot generate new salt\n");
        return FANTOM_FAIL;
    }

    char *password_hash = hash_password(new_password, salt);

    int rc = sqlite3_bind_text(fdb->change_password_stmt,
                               1,
                               password_hash,
                               strlen(password_hash),
                               SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        lprintf(LOG_ERROR, "Cannot bind change password stmt (hash) %s\n", sqlite3_errmsg(fdb->db));
        sqlite3_reset(fdb->change_password_stmt);
        free(password_hash);
        return FANTOM_FAIL;
    }

    rc = sqlite3_bind_text(fdb->change_password_stmt,
                           2,
                           salt,
                           strlen(salt),
                           SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        lprintf(LOG_ERROR, "Cannot bind change password stmt (salt) %s\n", sqlite3_errmsg(fdb->db));
        sqlite3_reset(fdb->change_password_stmt);
        free(password_hash);
        free(salt);
        return FANTOM_FAIL;
    }

    rc = sqlite3_bind_int(fdb->change_password_stmt,
                          3,
                          uid);
    if (rc != SQLITE_OK) {
        lprintf(LOG_ERROR, "Cannot bind change password stmt (uid) %s\n", sqlite3_errmsg(fdb->db));
        sqlite3_reset(fdb->change_password_stmt);
        free(password_hash);
        free(salt);
        return FANTOM_FAIL;
    }

    rc = sqlite3_step(fdb->change_password_stmt);

    fantom_status_t s = rc == SQLITE_DONE ? FANTOM_SUCCESS : FANTOM_FAIL;
    if (s == FANTOM_FAIL) {
        lprintf(LOG_ERROR, "Cannot change password %s\n", sqlite3_errmsg(fdb->db));
    }

    sqlite3_reset(fdb->change_password_stmt);
    free(password_hash);
    free(salt);

    return s;
}

