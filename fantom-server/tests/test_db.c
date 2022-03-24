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
}

int test_db()
{
    unit_test tests[] = {
				{&test_default_pass, "test_default_pass"},
				{&test_init_free, "test_init_free"}
    };

	  return run_tests(tests, TESTS_SIZE(tests), "db.c");
}
