#include "./get_nonce.h"
#include "../utils.h"
#include "../security.h"

void get_nonce_enp(struct mg_connection *c, fantom_server_t s)
{
    unsigned int ret;
    fantom_status_t t = get_nonce(s.nonce_mgr, &ret);

    if (t) {
        mg_http_reply(c, 200, NULL, "{\"nonce\":%u}", ret);
    } else {
        char *msg = get_error_msg("503 - Nonce service busy.");
        if (msg == NULL) {
            mg_http_reply(c, 503, "Retry-After: 5\r\n", "err");
        } else {
            mg_http_reply(c, 503, "Retry-After: 5\r\n", msg);
            free(msg);
        }
    }
}

