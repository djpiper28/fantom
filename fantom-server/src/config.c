#include <stdlib.h>
#include <jansson.h>
#include <string.h>
#include "config.h"
#include "logger.h"

void fantom_config_help()
{
    lprintf(LOG_INFO, "Your configuration file i.e: %s must be in the following format:\n%s\n", CONFIG_FILE_NAME, CONFIG_HELP);

    FILE *f = fopen(CONFIG_FILE_NAME, "w");
    if (f == NULL) {
        lprintf(LOG_ERROR, "Cannot write to config file %s\n", CONFIG_FILE_NAME);
    } else {
        fprintf(f, "%s", CONFIG_HELP);
        fclose(f);
        lprintf(LOG_INFO, "An example configuration file was made called %s\n", CONFIG_FILE_NAME);
    }
}

fantom_status_t fantom_init_config(FILE *input, fantom_config_t *output)
{
    if (input == NULL) {
        lprintf(LOG_ERROR, "Config file is unreadable\n");
        return FANTOM_FAIL;
    }

    // Zero the config and, init the state
    memset(output, 0, sizeof(*output));

    char *db_file = NULL;
    char *bind_url = NULL;
    int max_log_age_days = -1;

    // Read form file
    size_t buffer_length = BUFFER_LENGTH;
    size_t buffer_pointer = 0;
    char *ptr = malloc(sizeof * ptr * buffer_length);

    // Read to buffer
    for (int c = EOF; (c = fgetc(input)) != EOF && ptr != NULL;) {
        // Grow buffer if needed
        if (buffer_pointer + 1 >= buffer_length) {
            buffer_length += BUFFER_LENGTH;
            char *next_ptr = realloc(ptr, buffer_length);

            if (next_ptr == NULL) {
                free(ptr);
                ptr = NULL;
            } else if (next_ptr != ptr) {
                ptr = next_ptr;
            }

        }

        // Check for errors in buffer growth
        if (ptr != NULL) {
            ptr[buffer_pointer++] = (char) c;
        }
    }

    fantom_status_t status = FANTOM_SUCCESS;

    // Finalise read and, cleanup tmp
    if (ptr != NULL) {
        // Parse the json
        json_t *root;
        json_error_t error;
        root = json_loads(ptr, 0, &error);

        free(ptr);

        // Json to internal structs
        if (!root) {
            status = FANTOM_FAIL;
            lprintf(LOG_ERROR, "Cannot parse config file as json\n");
        } else if (!json_is_object(root)) {
            status = FANTOM_FAIL;
            lprintf(LOG_ERROR, "Config file config file is not in expected form\n");
            json_decref(root);
        } else {
            json_t *jtmp;

            jtmp = json_object_get(root, "db_file");
            if (json_is_string(jtmp)) {
                const char * t = json_string_value(jtmp);
                size_t len = strlen(t);

                db_file = malloc(sizeof * db_file * (len + 1));
                strcpy(db_file, t);
            } else {
                lprintf(LOG_ERROR, "db_file is not a number in the config file\n");
            }

            jtmp = json_object_get(root, "bind_url");
            if (json_is_string(jtmp)) {
                const char * t = json_string_value(jtmp);
                size_t len = strlen(t);

                bind_url = malloc(sizeof * db_file * (len + 1));
                strcpy(bind_url, t);
            } else {
                lprintf(LOG_ERROR, "bind_url is not a number in the config file\n");
            }

            jtmp = json_object_get(root, "max_log_age_days");
            if (json_is_number(jtmp)) {
                max_log_age_days = json_number_value(jtmp);
            } else {
                lprintf(LOG_ERROR, "max_log_age_days is not an integer in the config file\n");
            }

            json_decref(root);
        }
    }

    if (status) {
        status = db_file != NULL && bind_url != NULL && max_log_age_days > 0;
    }

    if (db_file != NULL) {
        if (status) {
            output->db_file = db_file;
        } else {
            free(db_file);
        }
    } else {
        lprintf(LOG_ERROR, "db_file is not defined\n");
    }

    if (bind_url != NULL) {
        if (status) {
            output->bind_url = bind_url;
        }	else {
            free(bind_url);
        }
    } else {
        lprintf(LOG_ERROR, "bind_url is not defined\n");
    }

    if (max_log_age_days <= 0 ) {
        lprintf(LOG_ERROR, "max_log_age_days is not defined or below or equal to zero\n");
    }

    // Check for errors
    if (!status) {
        lprintf(LOG_ERROR, "Error parsing the config file\n");
        fantom_free_config(output);
    } else {
        output->max_log_age_days = max_log_age_days;
        // The other members are set above to free the tmp vars if the config is not valid
    }

    return status;
}

fantom_status_t fantom_free_config(fantom_config_t *config)
{
    if (config->db_file != NULL) {
        free(config->db_file);
        config->db_file = NULL;
    }

    if (config->bind_url != NULL) {
        free(config->bind_url);
        config->bind_url = NULL;
    }

    return FANTOM_SUCCESS;
}

