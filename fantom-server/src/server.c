#include <string.h>
#include <signal.h>
#include "mongoose.h"
#include "logger.h"
#include "server.h"
#include "security.h"
#include "utils.h"
#include "enp/get_nonce.h"
#include "enp/login.h"

#define PREFLIGHT_METHOD "OPTIONS"
#define HTTP_HEADER_END "\r\n"
#define PREFLIGHT_HEADERS "Access-Control-Allow-Origin: *" HTTP_HEADER_END \
                          "Access-Control-Allow-Headers: *" HTTP_HEADER_END
// \r\n for HTTP right????!?

#ifdef TEST
int server_running_override = 1;
#endif

static int running = 1;
static void signal_handler(int signo)
{
    running = 0;
    lprintf(LOG_WARNING, "SIG %d received, terminating...\n", signo);
}

fantom_status_t check_nonce(struct mg_connection *c, fantom_server_t s, struct mg_http_message *hm)
{
    struct mg_str *str = mg_http_get_header(hm, "Nonce");
    if (str == NULL) {
        return FANTOM_FAIL;
    } else {
        char *buffer = malloc(str->len + 1);
        if (buffer == NULL) {
            lprintf(LOG_ERROR, "Malloc error\n");
            return FANTOM_FAIL;
        }

        strncpy(buffer, str->ptr, str->len + 1);
        unsigned int val = (unsigned int) atol(buffer);
        free(buffer);

        fantom_status_t ret = use_nonce(s.nonce_mgr, val);
        return ret;
    }
}

void send_500_error(struct mg_connection *c)
{
    char *msg = get_error_msg("500 - Internal server error");
    if (msg == NULL) {
        mg_http_reply(c, 500, "", "%S", "err");
    } else {
        mg_http_reply(c, 500, "", "%s", msg);
        free(msg);
    }
}

void send_400_error(struct mg_connection *c)
{
    char *msg = get_error_msg("400 - Bad input");
    if (msg == NULL) {
        mg_http_reply(c, 400, "", "%s", "err");
    } else {
        mg_http_reply(c, 400, "", "%s", msg);
        free(msg);
    }
}

void send_403_error(struct mg_connection *c)
{
    char *msg = get_error_msg("403 - Not authorised");
    if (msg == NULL) {
        mg_http_reply(c, 403, "", "%s", "err");
    } else {
        mg_http_reply(c, 403, "", "%s", msg);
        free(msg);
    }
}

static void protected_route(struct mg_connection *c,
                            fantom_server_t s,
                            struct mg_http_message *hm,
                            void (*fn)(struct mg_connection *, fantom_server_t, struct mg_http_message *))
{
    fantom_status_t r = check_nonce(c, s, hm);
    if (r == FANTOM_SUCCESS) {
        fn(c, s, hm);
    } else {
        char *msg = get_error_msg("400 - Invalid nonce");
        if (msg == NULL) {
            mg_http_reply(c, 400, NULL, "err");
        } else {
            mg_http_reply(c, 400, NULL, msg);
            free(msg);
        }
    }
}

static void cb(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = ev_data;
        fantom_server_t s = *(fantom_server_t *) fn_data;

        char uri[256];
        size_t len = MIN(sizeof(uri), hm->uri.len);
        strncpy(uri, hm->uri.ptr, len);
        uri[len] = 0;

        char ip_addr[max(sizeof("255.255.255.255"), sizeof("ff:ff:ff:ff:ff:ff"))];
        mg_ntoa(&c->rem, ip_addr, sizeof(ip_addr));

        lprintf(LOG_INFO, "Access request %s%s\n", ip_addr, uri);

        // Check for chrome preflights
        if (strncmp(hm->method.ptr, PREFLIGHT_METHOD, min(hm->method.len, sizeof(PREFLIGHT_METHOD))) == 0) {
            mg_http_reply(c, 200, PREFLIGHT_HEADERS, "y");
        }

        // Route the requests
        else if (mg_http_match_uri(hm, "/api/get_nonce")) {
            get_nonce_enp(c, s);
        } else {
            // Protected routes
            if (mg_http_match_uri(hm, "/api/login")) {
                protected_route(c, s, hm, &login_enp);
            } else {
                char *msg = get_error_msg("404 - Page not found");
                if (msg == NULL) {
                    mg_http_reply(c, 404, "", "err");
                } else {
                    mg_http_reply(c, 404, "", "%s", msg);
                    free(msg);
                }
            }
        }
    }
}

void start_fantom_server(fantom_config_t *config, fantom_db_t *db)
{
    lprintf(LOG_INFO, "Starting server on %s with the database file %s and, max log age of %d\n", config->bind_url, config->db_file, config->max_log_age_days);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    fantom_nonce_manager_t nonce_mgr;
    init_nonce_manager(&nonce_mgr);

    fantom_server_t fantom_server = {config, db, &nonce_mgr};
    struct mg_mgr mgr;
    mg_log_set(MG_DEBUG_LVL);
    mg_mgr_init(&mgr);

    struct mg_connection *c = mg_http_listen(&mgr, config->bind_url, cb, &fantom_server);
    if (c == NULL) {
        lprintf(LOG_ERROR, "Cannot start server on %s\n", config->bind_url);
        return;
    }

#ifdef TEST
    while(running && server_running_override) {
#else
    while (running) {
#endif
        mg_mgr_poll(&mgr, 10);
    }

    lprintf(LOG_INFO, "Stopping server...\n");
    free_nonce_manager(&nonce_mgr);
    mg_mgr_free(&mgr);
}

