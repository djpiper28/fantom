#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

static int test_nonce_manager_deadlocks()
{
    fantom_nonce_manager_t mgr;
    init_nonce_manager(&mgr);

		// Test for deadlocks in get nonce
    unsigned int r;
    for (int i = 0; i < 1000; i++) {
        get_nonce(&mgr, &r);
    }

		// Wait for a few rounds of polling
    for (int i = NONCE_TIMEOUT_S + 2; i > 0; i--) {
        lprintf(TEST_INFO, "Waiting for nonce manager %d...\n", i);
        sleep(1);
    }

		// Test for deadlocks after polling
    for (int i = 0; i < 1000; i++) {
        get_nonce(&mgr, &r);
    }

    lprintf(TEST_INFO, "Locking...\n");
    pthread_mutex_lock(&mgr.lock_var);
    pthread_mutex_unlock(&mgr.lock_var);
    lprintf(TEST_INFO, "Unlocked\n");

    free_nonce_manager(&mgr);
    return 1;
}

int test_security()
{
    unit_test tests[] = {
        {&test_init_seed, "test_init_seed"},
        {&test_get_salt, "test_get_salt"},
        {&test_hash_password, "test_hash_password"},
        {&test_nonce_manager_deadlocks, "test_nonce_manager_deadlocks"}
    };

    return run_tests(tests, TESTS_SIZE(tests), "security.c");
}

