#include "./testing.h"
#include "../src/logger.h"
#include "../src/banner.h"
#include "./test_db.h"
#include "./test_config.h"
#include "./test_utils.h"
#include "./test_security.h"
#include "./test_server.h"
#include "./sys_tests.h"
#include "./monitoring/test_ram.h"
#include "./monitoring/test_cpu.h"

int main()
{
    print_intro();
    lprintf(LOG_INFO, "Running unit tests\n");

    int failed = 0;

    failed += test_db();
    failed += test_config();
    failed += test_utils();
    failed += test_security();
    failed += test_mon_ram();
    failed += test_mon_cpu();
    failed += test_server();
    failed += system_tests();

    if (failed == 0) {
        lprintf(LOG_INFO, "All unit tests passed\n");
    } else {
        lprintf(LOG_ERROR, "Some unit tests failed\n");
    }
    return failed == 0 ? 0 : 1;
}
