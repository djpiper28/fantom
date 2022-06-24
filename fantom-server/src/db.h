#pragma once
#include <sqlite3.h>
#include "fantom_utils.h"

#define DEFAULT_USER "admin"
#define DEFAULT_PASSWORD_SALT ""
#define DEFAULT_PASSWORD_HASH "2ab7f298204c8588cd304395d08d4baa855b1baedc5db54031b0901533f9cc77d6cb33823587f96984ea57f069a4c15952c5043e0ab7aa53669f2b81ff75268f"
#define DEFAULT_PASSWORD "@dM1n"

typedef struct fantom_db_t {
    sqlite3 *db;
} fantom_db_t;

// Opens the db, if it is not present then it will make one and, create the tables.
fantom_status_t init_db(fantom_db_t *fdb, char *db_file);
void free_db(fantom_db_t *db);

// Database model
typedef enum fantom_user_status_t {
    FANTOM_USER_ACCOUNT_LOCKED = -1,
    FANTOM_USER_PASSWORD_NEEDS_CHANGE = 0,
    FANTOM_USER_VALID = 1
} fantom_user_status_t;

typedef struct fantom_user_t {
    int uid;
    char *name;
    fantom_user_status_t status;
} fantom_user_t;

void free_user(fantom_user_t *user);

typedef struct fantom_users_t {
    size_t length;
    fantom_user_t *users;
} fantom_users_t;

void free_users(fantom_users_t *users);

fantom_status_t db_get_user(fantom_db_t *fdb, int uid, fantom_user_t *ret);
fantom_status_t db_get_all_users(fantom_db_t *fdb, fantom_users_t *ret);
fantom_status_t db_login(fantom_db_t *fdb, char *name, char *password, fantom_user_t *ret);

int db_is_user_admin(fantom_db_t *fdb, int uid);

