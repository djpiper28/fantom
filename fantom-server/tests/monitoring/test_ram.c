#include "../testing.h"
#include "../src/logger.h"
#include "../src/monitoring/ram.h"

static int test_ram_read_info()
{
    fantom_ram_record_t inf;
    fantom_status_t c = read_mem_info(&inf);

    ASSERT(c == FANTOM_SUCCESS);
    ASSERT(inf.ram_total > 0);
    ASSERT(inf.ram_free > 0);
    ASSERT(inf.ram_total > inf.ram_free);

    ASSERT(inf.swap_total > 0);
    ASSERT(inf.swap_free > 0);
    ASSERT(inf.swap_total > inf.swap_free);

    ASSERT(inf.time > 0);

    lprintf(LOG_INFO, "Memory Used %ld %%\n", (100 * (inf.ram_total - inf.ram_free)) / inf.ram_total);
    lprintf(LOG_INFO, "Swap   Used %ld %%\n", (100 * (inf.swap_total - inf.swap_free)) / inf.swap_total);
    return 1;
}

int test_mon_ram()
{
    unit_test tests[] = {
        {&test_ram_read_info, "test_ram_read_info"}
    };

    return run_tests(tests, TESTS_SIZE(tests), "monitoring/ram.c");
}

