#include <stdio.h>
#include "test_server.h"
#include "testing.h"
#include "../src/security.h"
#include "../src/server.h"
#include "../src/logger.h"

static int test_protected_route()
{
    fantom_nonce_manager_t mgr;
    fantom_status_t s = init_nonce_manager(&mgr);
    ASSERT(s == FANTOM_SUCCESS);

    unsigned int nonce;
    s = get_nonce(&mgr, &nonce);
    ASSERT(s == FANTOM_SUCCESS);

    struct mg_connection *c = (struct mg_connection *) 1;
    struct mg_http_message hm;

    char bfr[128];
    snprintf(bfr, sizeof(bfr), "%u\r\n\r\n", nonce);
    struct mg_http_header header;

    header.name.ptr = "Nonce";
    header.name.len = strlen(header.name.ptr);
    header.value.ptr = bfr;
    header.value.len = strlen(bfr);

    hm.headers[0] = header;

    struct mg_str *str = mg_http_get_header(&hm, "Nonce");
    ASSERT(str != NULL);

    fantom_server_t srv;
    srv.nonce_mgr = &mgr;

    s = check_nonce(c, srv, &hm);
    ASSERT(s == FANTOM_SUCCESS);

    free_nonce_manager(&mgr);
    return 1;
}

static int test_protected_route_fail()
{
    fantom_nonce_manager_t mgr;
    fantom_status_t s = init_nonce_manager(&mgr);
    ASSERT(s == FANTOM_SUCCESS);

    struct mg_connection *c = (struct mg_connection *) 1;
    struct mg_http_message hm;

    struct mg_http_header header;

    header.name.ptr = "Nonce";
    header.name.len = strlen(header.name.ptr);
    header.value.ptr = "123\r\n\r\n";
    header.value.len = strlen(header.value.ptr);

    hm.headers[0] = header;

    struct mg_str *str = mg_http_get_header(&hm, "Nonce");
    ASSERT(str != NULL);

    fantom_server_t srv;
    srv.nonce_mgr = &mgr;

    s = check_nonce(c, srv, &hm);
    ASSERT(s == FANTOM_FAIL);

    free_nonce_manager(&mgr);
    return 1;
}


int test_server()
{
    unit_test tests[] = {
        {&test_protected_route, "test_protected_route"},
        {&test_protected_route_fail, "test_protected_route_fail"}
    };

    return run_tests(tests, TESTS_SIZE(tests), "server.c");
}

