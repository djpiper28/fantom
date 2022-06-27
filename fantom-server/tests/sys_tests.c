#include <pthread.h>
#include <jansson.h>
#include <stdlib.h>
#include <unistd.h>
#include "./sys_tests.h"
#include "./testing.h"
#include "./curl_utils.h"
#include "../src/main.h"
#include "../src/server.h"
#include "../src/db.h"
#include "../src/config.h"

static void *start_server(void *data)
{
    fantom_main(0, NULL);
    pthread_exit(NULL);
    return NULL;
}

static pthread_t server;

static int init_sys_tests()
{
    system("bash -c \"rm -f *.json rm *.db\"");
    fantom_main(0, NULL); // init setup

    pthread_create(&server, NULL, &start_server, NULL);

    for (int i = 2; i > 0; i--) {
        lprintf(LOG_INFO, "Waiting %d for server to start...\n", i);
        sleep(1);
    }

    return 1;
}

static int test_404()
{
    char *str = get_request(CONFIG_DEFAULT_BIND "/adkaosjdiuahdiau");
    ASSERT(str != NULL);
    ASSERT(strlen(str) > 0);
    free(str);
    return 1;
}

static int test_preflight()
{
    char *ret = send_request(CONFIG_DEFAULT_BIND "/odnaosdasd", "", "OPTIONS", 123);
    ASSERT(ret != NULL);
    free(ret);
    return 1;
}

static int test_get_nonce()
{
    char *nonce_str = get_request(CONFIG_DEFAULT_BIND "/api/get_nonce");
    ASSERT(nonce_str != NULL);

    unsigned int nonce;
    int found = 0;

    // Parse the json
    json_t *root;
    json_error_t error;
    root = json_loads(nonce_str, 0, &error);

    free(nonce_str);

    // Json to internal structs
    if (!root) {
        lprintf(LOG_ERROR, "Cannot parse config file as json\n");
        return 0;
    } else if (!json_is_object(root)) {
        lprintf(LOG_ERROR, "Config file config file is not in expected form\n");
        json_decref(root);
        return 0;
    } else {
        json_t *jtmp;

        jtmp = json_object_get(root, "nonce");
        if (json_is_number(jtmp)) {
            nonce = (unsigned int) json_number_value(jtmp);
            found = 1;
        } else {
            lprintf(LOG_ERROR, "No numerical nonce\n");
            return 0;
        }

        json_decref(root);
    }

    ASSERT(found);
    lprintf(LOG_INFO, "Nonce is %u\n", nonce);
    return 1;
}

static int test_login()
{
    char *nonce_str = get_request(CONFIG_DEFAULT_BIND "/api/get_nonce");
    ASSERT(nonce_str != NULL);

    unsigned int nonce;
    int found = 0;

    // Parse the json
    json_t *root;
    json_error_t error;
    root = json_loads(nonce_str, 0, &error);

    free(nonce_str);

    // Json to internal structs
    if (!root) {
        lprintf(LOG_ERROR, "Cannot parse config file as json\n");
        return 0;
    } else if (!json_is_object(root)) {
        lprintf(LOG_ERROR, "is not in expected form\n");
        json_decref(root);
        return 0;
    } else {
        json_t *jtmp;

        jtmp = json_object_get(root, "nonce");
        if (json_is_number(jtmp)) {
            nonce = (unsigned int) json_number_value(jtmp);
            found = 1;
        } else {
            lprintf(LOG_ERROR, "No numerical nonce\n");
            return 0;
        }

        json_decref(root);
    }

    ASSERT(found);
    lprintf(LOG_INFO, "Nonce is %u\n", nonce);

    json_t *obj = json_pack("{ssss}", "name", DEFAULT_USER,
                            "password", DEFAULT_PASSWORD);
    ASSERT(obj != NULL);

    char *json = json_dumps(obj, JSON_ENCODE_ANY);
    json_decref(obj);
    ASSERT(json != NULL);

    char *ret = send_request(CONFIG_DEFAULT_BIND "/api/login", json, "GET", nonce);
    ASSERT(ret != NULL);

    root = json_loads(ret, 0, &error);

    free(json);
    free(ret);

    found = 0;
    int status;
    int jwt;

    // Json to internal structs
    if (!root) {
        lprintf(LOG_ERROR, "Cannot parse config file as json\n");
        return 0;
    } else if (!json_is_object(root)) {
        lprintf(LOG_ERROR, "is not in expected form\n");
        json_decref(root);
        return 0;
    } else {
        json_t *jtmp;

        jtmp = json_object_get(root, "status");
        if (json_is_number(jtmp)) {
            status = (unsigned int) json_number_value(jtmp);
            found = 1;
        } else {
            lprintf(LOG_ERROR, "No numerical status\n");
            return 0;
        }

        jtmp = json_object_get(root, "jwt");
        if (json_is_string(jtmp)) {
            jwt = 1;
        }

        json_decref(root);
    }

    ASSERT(found);
    ASSERT(status == FANTOM_USER_PASSWORD_NEEDS_CHANGE);
    ASSERT(jwt);

    return 1;
}

static int cleanup_sys_tests()
{
    server_running_override = 0;

    void *r;
    pthread_join(server, &r);

    return 1;
}

int system_tests()
{
    unit_test tests[] = {
        {&init_sys_tests, "init_sys_tests"},
        {&test_404, "test_404"},
        {&test_preflight, "test_preflight"},
        {&test_get_nonce, "test_get_nonce"},
        {&test_login, "test_login"},
        {&cleanup_sys_tests, "cleanup_sys_tests"}
    };

    return run_tests(tests, TESTS_SIZE(tests), "system testss");
}

