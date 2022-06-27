#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "test_utils.h"
#include "testing.h"
#include "../src/logger.h"
#include "../src/utils.h"

static int test_bad_read()
{
    ASSERT(read_file(NULL) == NULL);
    return 1;
}

static int test_good_read()
{
    int fid[2];
    pipe(fid);

    FILE *r = fdopen(fid[0], "r");
    FILE *w = fdopen(fid[1], "w");
    ASSERT(r != NULL);
    ASSERT(w != NULL);

    fprintf(w, "%s", TEST_UTILS_STR);
    fclose(w);

    char *ret = read_file(r);
    ASSERT(ret != NULL);

    ASSERT(strcmp(ret, TEST_UTILS_STR) == 0);

    free(ret);
    fclose(r);

    return 1;
}

static int test_big_read()
{
    FILE *f = fopen("fantom-tests", "rb");
    ASSERT(f != NULL);

    char *ret = read_file(f);
    ASSERT(ret != NULL);

    free(ret);
    fclose(f);
    return 1;
}

static int test_get_error_msg()
{
    char * msg = get_error_msg("test");
    ASSERT(msg != NULL);
    free(msg);
    return 1;
}

int test_utils()
{
    unit_test tests[] = {
        {&test_bad_read, "test_bad_read"},
        {&test_good_read, "test_good_read"},
        {&test_big_read, "test_big_read"},
        {&test_get_error_msg, "test_get_error_msg"}
    };

    return run_tests(tests, TESTS_SIZE(tests), "utils.c");
}
