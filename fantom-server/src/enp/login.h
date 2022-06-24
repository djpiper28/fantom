#pragma once
#include "../mongoose.h"
#include "../server.h"

void login_enp(struct mg_connection *c, fantom_server_t s, struct mg_http_message *hm);

