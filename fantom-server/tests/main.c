#include "testing.h"
#include "../src/logger.h"
#include "../src/banner.h"
#include "test_db.h"

int main()
{
    print_intro();
    lprintf(LOG_INFO, "Running unit tests\n");

    test_db();
}
