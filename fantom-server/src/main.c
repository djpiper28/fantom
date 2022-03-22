#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "logger.h"
#include "config.h"
#include "banner.h"
#include "cli_help.h"

void print_intro()
{
	  lprintf(LOG_INFO, "F@ntom Version: %s. For: %s\n%s\n", VERSION, OS, BANNER);
	  lprintf(LOG_INFO, "A light-weight remote monitoring system for your machines.\n");
	  lprintf(LOG_INFO, "See ./README.md for help or, view the wiki.\n");
	  lprintf(LOG_INFO, "More information at: %s\n", REPO_URL);
}

void print_cli_help()
{
    print_intro();
	  lprintf(LOG_INFO, "F@ntom cli arguments help.\n%s\n", CLI_HELP_MESSAGE);
	  printf("Logs and, help are printed to stderr.\n");
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

void start_fantom_server(fantom_config_t config)
{
	  lprintf(LOG_INFO, "Starting server on %s with the database as %s.\n", config.bind_url, config.db_file);
}

// The bootstrapper - loads the config and, checks the db for validity
void start_fantom(const char *config_file) {
    fantom_config_t config;
    if (!load_config(&config, config_file)) {
        return;
    }

    lprintf(LOG_INFO, "Read configuration file successfully\n");

    // Call start server
    start_fantom_server(config);
    
    fantom_free_config(&config);

		return;
}

int main (int argc, char **argv)
{
    // Parse CLI args
    int err = 0;
    for (int i = 1; i < argc; i++) {
    	  if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
    	  	  print_cli_help();
    	  	  return 1;
    	  } else {
    	  	  err = 1;
    	  }
    }

    if (err) {
       lprintf(LOG_ERROR, "Unable to parse CLI arguments, aborting.\n");
       print_cli_help();
       return 1;
    }

		print_intro();
    lprintf(LOG_INFO, "Starting F@ntom...\n");    
    start_fantom(CONFIG_FILE_NAME);
    lprintf(LOG_ERROR, "Fantom has terminated.\n");

    return 2;
}
