#include "testing.h"
#include "../src/logger.h"
#include "../src/banner.h"
#include "test_db.h"
#include "test_config.h"
#include "test_utils.h"

int main()
{
    print_intro();
    lprintf(LOG_INFO, "Running unit tests\n");

    int failed = 0;

    failed += test_db();
    failed += test_config();
    failed += test_utils();

    if (failed == 0) {
        lprintf(LOG_INFO, "All unit tests passed\n");
    } else {
        lprintf(LOG_ERROR, "Some unit tests failed\n");
    }
    return failed == 0 ? 0 : 1;
}
