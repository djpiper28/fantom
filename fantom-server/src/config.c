#include "config.h"
#include "logger.h"

void fantom_config_help()
{
    const char *json_help = "{\n"
    				"  \"db_name\": \"fantom\"\n"
    				"  \"db_username\": \"username\"\n"
    				"  \"db_password\": \"password\"\n"
    				"  \"db_url\": \"localhost:5321\"\n"
    				"  \"bind_url\": \"http://127.0.0.1:8765\"\n"
    				"  \"max_log_age_days\": 5\n"
    				"}\n";
    lprintf(LOG_INFO, "Your configuration file i.e: %s must be in the following format:\n", CONFIG_FILE_NAME);
    lprintf(LOG_INFO, "%s\n", json_help);
    lprintf(LOG_INFO, "An example file was made called %s\n", CONFIG_FILE_NAME);
}

fantom_status_t fantom_init_config(FILE *input, fantom_config_t *output)
{
	
}

fantom_status_t fantom_free_config(fantom_config_t *config)
{
	
}

