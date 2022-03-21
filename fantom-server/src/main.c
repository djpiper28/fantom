#include <stdlib.h>
#include <stdio.h>
#include "logger.h"

int main (int argc, char **argv)
{
    lprintf(LOG_INFO, "Starting fantom...\n");

    lprintf(LOG_ERROR, "Fantom has terminated.\n");
}
