#pragma once
#include "../server.h"
#include "../mongoose.h"

fantom_status_t try_change_password(fantom_server_t s, char *name, char *old_password, char *new_password);
void change_password_enp(struct mg_connection *c, fantom_server_t s, struct mg_http_message *hm);
