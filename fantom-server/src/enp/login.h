#pragma once
#include "../mongoose.h"
#include "../server.h"
#include "../db.h"

typedef struct fantom_login_details_t {
    fantom_user_t user;
    char *jwt;
} fantom_login_details_t;

void free_login_details(fantom_login_details_t *d);
fantom_status_t try_login(fantom_server_t s, char *name, char *password, fantom_login_details_t *ret);
void login_enp(struct mg_connection *c, fantom_server_t s, struct mg_http_message *hm);

