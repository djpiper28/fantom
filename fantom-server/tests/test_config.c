#include <stdio.h>
#include "test_config.h"
#include "testing.h"
#include "../src/logger.h"
#include "../src/config.h"

static int test_init_free()
{
    fantom_config_t config;

    ASSERT(fantom_init_config(NULL, &config) == FANTOM_FAIL);
    fantom_free_config(&config);
    return 1;
}

static int test_make_default_config()
{
    fantom_config_t config;

    // Create the config file
    fantom_config_help();

    // A default config should have been made now
    FILE *f = fopen(CONFIG_FILE_NAME, "r");
    ASSERT(f != NULL);
    ASSERT(fantom_init_config(f, &config) == FANTOM_SUCCESS);

    fclose(f);
    fantom_free_config(&config);
    return 1;
}

int test_config()
{
    unit_test tests[] = {
        {&test_init_free, "test_init_free"},
        {&test_make_default_config, "test_make_default_config"}
    };

    return run_tests(tests, TESTS_SIZE(tests), "config.c");
}

