#include "../testing.h"
#include "../src/logger.h"
#include "../src/monitoring/cpu.h"

static int test_cpu_read_info()
{
    fantom_cpu_record_t inf;
    fantom_status_t c = read_cpu_info(&inf);

    ASSERT(c == FANTOM_SUCCESS);

    ASSERT(inf.cores > 0);
    ASSERT(inf.val > 0.0);
    ASSERT(((double) inf.cores * 100) - inf.val >= -0.001);
    ASSERT(inf.time > 0);

    lprintf(LOG_INFO, "Load average: %f %%\n", inf.val / ((double) inf.cores * 100));

    return 1;
}

int test_mon_cpu()
{
    unit_test tests[] = {
        {&test_cpu_read_info, "test_cpu_read_info"}
    };

    return run_tests(tests, TESTS_SIZE(tests), "monitoring/cpu.c");
}

