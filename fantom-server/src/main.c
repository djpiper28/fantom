#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "db.h"
#include "logger.h"
#include "config.h"
#include "server.h"
#include "banner.h"
#include "cli_help.h"
#include "mongoose.h"
#include "security.h"

void print_cli_help()
{
    print_intro();
    lprintf(LOG_INFO, "F@ntom cli arguments help.\n%s\n", CLI_HELP_MESSAGE);
    printf("Logs and, help are printed to stderr.\n");
}

int load_database(fantom_db_t *db, fantom_config_t *config)
{
    fantom_status_t status = init_db(db, config->db_file);
    return status;
}

int load_config(fantom_config_t *config, char *filename)
{
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        lprintf(LOG_ERROR, "Cannot read configuration file '%s'\n", filename);
        fantom_config_help();
        return 0;
    }

    fantom_status_t s = fantom_init_config(f, config);
    fclose(f);

    if (!s) {
        lprintf(LOG_ERROR, "Configuration file is invalid\n");
        fantom_config_help();
        return 0;
    }

    return 1;
}

// The bootstrapper - loads the config and, checks the db for validity
void start_fantom(char *config_file)
{
    fantom_config_t config;
    if (!load_config(&config, config_file)) {
        return;
    }

    lprintf(LOG_INFO, "Read configuration file successfully\n");

    fantom_db_t db;
    if (!load_database(&db, &config)) {
        lprintf(LOG_ERROR, "Cannot open database\n");
        free_db(&db);
        return;
    }

    lprintf(LOG_INFO, "Opened the database file successfully\n");

    init_seed();

    // Call start server
    start_fantom_server(&config, &db);

    // Free the server then exit, this should never happen though :)
    fantom_free_config(&config);

    lprintf(LOG_INFO, "Closing database\n");
    free_db(&db);

    return;
}

#ifndef TEST
int main (int argc, char **argv)
#else
int fantom_main(int argc, char **argv)
#endif
{
    if (clearenv() != 0) {
        lprintf(LOG_ERROR, "Cannot clear environemnt variables");
        return 1;
    }

    // Parse CLI args
    int err = 0;
    char *config_file = CONFIG_FILE_NAME;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_cli_help();
            return 1;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) {
            if (i + 1 < argc) {
                config_file = argv[i++];
            } else {
                err = 1;
                lprintf(LOG_ERROR, "CLI argument -c (--config) has no config file specified\n");
            }
        } else {
            err = 1;
        }
    }

    if (err) {
        lprintf(LOG_ERROR, "Unable to parse CLI arguments, aborting\n");
        print_cli_help();
        return 1;
    }

    print_intro();
    lprintf(LOG_INFO, "Starting F@ntom...\n");
    start_fantom(config_file);
    lprintf(LOG_ERROR, "Fantom has terminated\n");

    return 2;
}
