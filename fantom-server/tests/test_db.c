#include <string.h>
#include "testing.h"
#include "../src/security.h"
#include "../src/db.h"

static int test_default_pass()
{
	  char *default_pwd_hash = hash_password(DEFAULT_PASSWORD, DEFAULT_PASSWORD_SALT);
	  ASSERT(default_pwd_hash != NULL);
	  
	  lprintf(TEST_INFO, "Password hashes: \ncalc: %s\n str: %s\n", default_pwd_hash, DEFAULT_PASSWORD_HASH);

	  ASSERT(strcmp(default_pwd_hash, DEFAULT_PASSWORD_HASH) == 0);
	  free(default_pwd_hash);

	  return 1;
}

int test_db()
{
    unit_test tests[] = {
				{&test_default_pass, "test_default_pass"}
    };

	  return run_tests(tests, TESTS_SIZE(tests), "db.c");
}
