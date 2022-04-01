#include "get_nonce.h"
#include "../security.h"

void get_nonce_enp(struct mg_connection *c, fantom_server_t s)
{
    unsigned int ret;
    fantom_status_t t = get_nonce(s.nonce_mgr, &ret);

    if (t) {
        mg_http_reply(c, 200, NULL, "{\"nonce\":%u}", ret);
    } else {
        mg_http_reply(c, 503, "Retry-After: 5\r\n", "503 - Nonce service busy.");
    }
}

