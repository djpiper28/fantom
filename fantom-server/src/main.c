#include <stdlib.h>
#include <stdio.h>
#include "logger.h"
#include "config.h"
#include "banner.h"

void print_intro()
{
	  lprintf(LOG_INFO, "Fantom %s for %s\n%s\n", VERSION, OS, BANNER);
	  lprintf(LOG_INFO, "See ./readme.md for help.\n");
	  lprintf(LOG_INFO, "More information at: %s\n", REPO_URL);
}

int load_config(fantom_config_t *config, char *filename)
{
	  FILE *f = fopen(filename, "r");
    if (f == NULL) {
    		lprintf(LOG_ERROR, "Cannot read configuration file '%s'.\n", filename);
    		fantom_config_help();
    		return 0;
    }

    fantom_status_t s = fantom_init_config(f, config);
	  fclose(f);

	  if (!s) {
	  	  lprintf(LOG_ERROR, "Configuration file is invalid.\n");
	  	  fantom_config_help();
	  	  return 0;
	  }

	  return 1;
}

void start_fantom(int argc, char **argv) {
    fantom_config_t config;
    if (!load_config(&config, CONFIG_FILE_NAME)) {
        return;
    }

    lprintf(LOG_INFO, "Read configuration file successfully\n");

    // Call start server
    
    fantom_free_config(&config);

		return;
}

int main (int argc, char **argv)
{
		print_intro();
    lprintf(LOG_INFO, "Starting fantom...\n");
    start_fantom(argc, argv);
    lprintf(LOG_ERROR, "Fantom has terminated.\n");
}
