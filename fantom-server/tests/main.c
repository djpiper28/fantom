#include "testing.h"
#include "../src/logger.h"
#include "../src/banner.h"

int main(int argc, char **argv)
{
    print_intro();
    lprintf(LOG_INFO, "Running unit tests\n");
}
