#include <string.h>
#include <unistd.h>
#include "testing.h"
#include "test_db.h"
#include "../src/security.h"
#include "../src/db.h"
#include "../src/fantom_utils.h"

static int test_default_pass()
{
    char *default_pwd_hash = hash_password(DEFAULT_PASSWORD, DEFAULT_PASSWORD_SALT);
    ASSERT(default_pwd_hash != NULL);

    lprintf(TEST_INFO, "Password hashes: \ncalc: %s\n str: %s\n", default_pwd_hash, DEFAULT_PASSWORD_HASH);

    ASSERT(strcmp(default_pwd_hash, DEFAULT_PASSWORD_HASH) == 0);
    free(default_pwd_hash);

    return 1;
}

static int test_init_free()
{
    fantom_db_t db;
    ASSERT(access(TEST_DB, F_OK) != 0);
    ASSERT(init_db(&db, TEST_DB) == FANTOM_SUCCESS);
    free_db(&db);

    ASSERT(access(TEST_DB, F_OK) == 0);
    ASSERT(init_db(&db, TEST_DB) == FANTOM_SUCCESS);
    free_db(&db);

    return 1;
}

static int test_get_admin_user()
{
    fantom_db_t db;
    ASSERT(access(TEST_DB, F_OK) == 0);
    ASSERT(init_db(&db, TEST_DB) == FANTOM_SUCCESS);

    fantom_user_t ret;
    fantom_status_t s = db_get_user(&db, 1, &ret);

    ASSERT(s == FANTOM_SUCCESS);
    ASSERT(ret.uid == 1);
    ASSERT(ret.name != NULL);
    ASSERT(strcmp(ret.name, DEFAULT_USER) == 0);
    ASSERT(ret.status == FANTOM_USER_PASSWORD_NEEDS_CHANGE);

    free_user(&ret);
    free_db(&db);

    return 1;
}

static int test_get_all_users()
{
    fantom_db_t db;
    ASSERT(access(TEST_DB, F_OK) == 0);
    ASSERT(init_db(&db, TEST_DB) == FANTOM_SUCCESS);

    fantom_users_t ret;
    fantom_status_t s = db_get_all_users(&db, &ret);

    ASSERT(s == FANTOM_SUCCESS);
    ASSERT(ret.length == 1);
    ASSERT(ret.users != NULL);

    fantom_user_t u = ret.users[0];

    ASSERT(u.uid == 1);
    ASSERT(u.name != NULL);
    ASSERT(strcmp(u.name, DEFAULT_USER) == 0);
    ASSERT(u.status == FANTOM_USER_PASSWORD_NEEDS_CHANGE);

    free_user(&u);
    free_users(&ret);
    free_db(&db);

    return 1;
}

static int test_login_admin()
{
    fantom_db_t db;
    ASSERT(access(TEST_DB, F_OK) == 0);
    ASSERT(init_db(&db, TEST_DB) == FANTOM_SUCCESS);

    fantom_user_t ret;
    fantom_status_t s = db_login(&db, DEFAULT_USER, DEFAULT_PASSWORD, &ret);

    ASSERT(s == FANTOM_SUCCESS);
    ASSERT(ret.uid == 1);
    ASSERT(ret.name != NULL);
    ASSERT(strcmp(ret.name, DEFAULT_USER) == 0);
    ASSERT(ret.status == FANTOM_USER_PASSWORD_NEEDS_CHANGE);

    free_user(&ret);
    free_db(&db);

    return 1;
}

static int test_login_admin_bad_password()
{
    fantom_db_t db;
    ASSERT(access(TEST_DB, F_OK) == 0);
    ASSERT(init_db(&db, TEST_DB) == FANTOM_SUCCESS);

    fantom_user_t ret;
    fantom_status_t s = db_login(&db, DEFAULT_USER, DEFAULT_PASSWORD DEFAULT_PASSWORD, &ret);

    ASSERT(s == FANTOM_FAIL);
    ASSERT(ret.uid == -1);
    ASSERT(ret.name == NULL);
    free_db(&db);

    return 1;
}

static int test_login_bad_user()
{
    fantom_db_t db;
    ASSERT(access(TEST_DB, F_OK) == 0);
    ASSERT(init_db(&db, TEST_DB) == FANTOM_SUCCESS);

    fantom_user_t ret;
    memset(&ret, 0, sizeof(ret));
    fantom_status_t s = db_login(&db, "jimmy", "asdfgh", &ret);

    ASSERT(s == FANTOM_FAIL);
    ASSERT(ret.uid == -1);
    ASSERT(ret.name == NULL);
    free_db(&db);

    return 1;
}

#define NEW_PASSWORD "73ondrO_li0_123"

static int test_change_password()
{
    fantom_db_t db;
    ASSERT(access(TEST_DB, F_OK) == 0);
    ASSERT(init_db(&db, TEST_DB) == FANTOM_SUCCESS);

    fantom_status_t s = db_change_password(&db, 1, NEW_PASSWORD);
    ASSERT(s == FANTOM_SUCCESS);

    fantom_user_t ret;
    s = db_login(&db, DEFAULT_USER, NEW_PASSWORD, &ret);

    ASSERT(s == FANTOM_SUCCESS);
    ASSERT(ret.uid == 1);
    ASSERT(ret.name != NULL);
    ASSERT(strcmp(ret.name, DEFAULT_USER) == 0);
    ASSERT(ret.status == FANTOM_USER_VALID);

    free_user(&ret);
    free_db(&db);

    return 1;
}

int test_db()
{
    unit_test tests[] = {
        {&test_default_pass, "test_default_pass"},
        {&test_init_free, "test_init_free"},
        {&test_get_admin_user, "test_get_user_for_admin"},
        {&test_get_all_users, "test_get_all_users"},
        {&test_login_admin, "test_login_admin"},
        {&test_login_admin_bad_password, "test_login_admin_bad_password"},
        {&test_login_bad_user, "test_login_bad_user"},
        {&test_change_password, "test_change_password"}
    };

    return run_tests(tests, TESTS_SIZE(tests), "db.c");
}
