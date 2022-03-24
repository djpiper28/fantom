#include "banner.h"
#include "logger.h"
#include "mongoose.h"

void print_intro()
{
    lprintf(LOG_INFO, "F@ntom Version: %s. For: %s - Mongoose v%s\n%s\n", VERSION, OS, MG_VERSION, BANNER);
    lprintf(LOG_INFO, "A light-weight remote monitoring system for your machines\n");
    lprintf(LOG_INFO, "See ./README.md for help or, view the wiki\n");
    lprintf(LOG_INFO, "More information at: %s\n", REPO_URL);
}

