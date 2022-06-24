#include <stdlib.h>
#include <string.h>
#include <jansson.h>
#include "get_nonce.h"
#include "../security.h"
#include "../db.h"
#include "../logger.h"
#include "../utils.h"

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
        char *err = get_error_msg("400 - \"name\" is not in the json object");

        if (err == NULL) {
            lprintf(LOG_ERROR, "Malloc error\n");
            send_500_error(c);
            return;
        }
        mg_http_reply(c, 400, NULL, err);
        free(err);
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

    int status = name != NULL || password != NULL;

    if (!status) {
        char *err = NULL;
        if (name == NULL) {
            err = get_error_msg("400 - \"name\" is not in the json object");
        } else if (password == NULL) {
            err = get_error_msg("400 - \"password\" is not in the json object");
        }

        if (err == NULL) {
            lprintf(LOG_ERROR, "Malloc error\n");
            send_500_error(c);
        } else {
            mg_http_reply(c, 400, NULL, err);
            free(err);
        }
    } else {
        fantom_user_t ret;
        fantom_status_t r = db_login(s.db, name, password, &ret);

        if (r == FANTOM_FAIL || ret.status == FANTOM_USER_ACCOUNT_LOCKED) {
            char *err = get_error_msg("403 - Cannot login");

            if (err == NULL) {
                lprintf(LOG_ERROR, "Malloc error\n");
                send_500_error(c);
                return;
            }
            mg_http_reply(c, 403, NULL, err);
            free(err);
            free_user(&ret);
            return;
        }

        char *jwt = issue_token(ret.uid, ret.name, s.conf->jwt_secret, s.conf);
        if (jwt == NULL) {
            lprintf(LOG_ERROR, "Malloc error\n");
            send_500_error(c);
            free_user(&ret);
            return;
        } else {
            json_t *obj = json_pack("{s:s;s:d}",
                                    "jwt", jwt,
                                    "status", ret.status);
            if (obj == NULL) {
                lprintf(LOG_ERROR, "Cannot encode json\n");
                send_500_error(c);
                free_user(&ret);
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
            free(jwt);
        }

        free_user(&ret);
    }
}

