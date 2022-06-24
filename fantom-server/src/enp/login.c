#include <stdlib.h>
#include <string.h>
#include <jansson.h>
#include "./login.h"
#include "../security.h"
#include "../logger.h"
#include "../utils.h"

void free_login_details(fantom_login_details_t *d)
{
    free_user(&d->user);
    if (d->jwt != NULL) {
        free(d->jwt);
    }
    d->jwt = NULL;
}

fantom_status_t try_login(fantom_server_t s, char *name, char *password, fantom_login_details_t *ret)
{
    fantom_user_t usr;
    fantom_status_t r = db_login(s.db, name, password, &usr);

    if (r == FANTOM_FAIL) {
        return FANTOM_FAIL;
    }

    if (usr.status == FANTOM_USER_ACCOUNT_LOCKED) {
        free_user(&usr);
        return FANTOM_FAIL;
    }

    char *jwt = issue_token(usr.uid, usr.name, s.conf->jwt_secret, s.conf);
    if (jwt == NULL) {
        free_user(&usr);
        return FANTOM_FAIL;
    }

    ret->user = usr;
    ret->jwt = jwt;
    return FANTOM_SUCCESS;
}

void login_enp(struct mg_connection *c, fantom_server_t s, struct mg_http_message *hm)
{
    char *body = malloc(hm->body.len + 1);
    if (body == NULL) {
        lprintf(LOG_ERROR, "Malloc error\n");
        send_500_error(c);
    }

    strncpy(body, hm->body.ptr, hm->body.len + 1);

    char *name = NULL;
    char *password = NULL;

    json_t *root;
    json_error_t error;
    root = json_loads(body, 0, &error);

    free(body);

    // Json to internal structs
    if (!root) {
        send_400_error(c);
        return;
    } else if (!json_is_object(root)) {
        json_decref(root);
        send_500_error(c);
        return;
    } else {
        json_t *jtmp;

        jtmp = json_object_get(root, "name");
        if (json_is_string(jtmp)) {
            const char * t = json_string_value(jtmp);
            size_t len = strlen(t);

            name = malloc(len + 1);
            strcpy(name, t);
        }

        jtmp = json_object_get(root, "password");
        if (json_is_string(jtmp)) {
            const char * t = json_string_value(jtmp);
            size_t len = strlen(t);

            password = malloc(len + 1);
            strcpy(password, t);
        }
        json_decref(root);
    }

    int status = name != NULL && password != NULL;

    if (!status) {
        send_400_error(c);
    } else {
        fantom_login_details_t d;
        fantom_status_t status = try_login(s, name, password, &d);
        if (status == FANTOM_FAIL) {
            send_403_error(c);
        } else {
            json_t *obj = json_pack("{sssi}",
                                    "jwt", d.jwt,
                                    "status", d.user.status);
            if (obj == NULL) {
                lprintf(LOG_ERROR, "Cannot encode json\n");
                send_500_error(c);
                free_login_details(&d);
                return;
            }

            char *msg = json_dumps(obj, JSON_ENCODE_ANY);
            json_decref(obj);

            if (msg == NULL) {
                send_500_error(c);
            } else {
                mg_http_reply(c, 200, NULL, msg);

                free(msg);
            }
        }

        free_login_details(&d);
    }
}

