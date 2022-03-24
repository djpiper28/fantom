#include <stdlib.h>
#include <string.h>
#include "testing.h"
#include "test_security.h"
#include "../src/logger.h"
#include "../src/security.h"

static int test_get_salt()
{
	  char *salt = get_salt();
	  ASSERT(salt != NULL);
	  ASSERT(strlen(salt) == SALT_LENGTH);

	  free(salt);
	  return 1;
}

static int test_hash_password()
{
	  char *salt = get_salt();
	  ASSERT(salt != NULL);
	  char *hash = hash_password(TEST_SECURITY_PWD, salt);
	  ASSERT(hash != NULL);
	  ASSERT(strlen(hash) == SHA512_DIGEST_STRING_LENGTH);
	  
    // Test repeatability
	  char *hash2 = hash_password(TEST_SECURITY_PWD, salt);
	  ASSERT(hash2 != NULL);
	  ASSERT(strcmp(hash, hash2) == 0);
	  ASSERT(strlen(hash2) == SHA512_DIGEST_STRING_LENGTH);
	  
	  free(salt);
	  free(hash);
	  free(hash2);
	  return 1;
}

static int test_init_seed()
{
	  init_seed();
	  return 1;
}

int test_security()
{
    unit_test tests[] = {
				{&test_init_seed, "test_init_seed"},
        {&test_get_salt, "test_get_salt"},
				{&test_hash_password, "test_hash_password"}
    };

    return run_tests(tests, TESTS_SIZE(tests), "security.c");
}

