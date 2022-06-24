#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "../src/logger.h"
#include "./curl_utils.h"

typedef struct curl_response {
    char *ptr;
    size_t len;
} curl_response;

static size_t write_callback(char *ptr_in,
                             size_t size,
                             size_t nmemb,
                             void *userdata)
{
    curl_response *response = (curl_response *) userdata;
    if (response->ptr == NULL) {
        response->ptr = (char *) malloc((size * nmemb) + 1);
        if (response->ptr == NULL) {
            lprintf(LOG_ERROR, "Malloc error\n");
            return 0;
        }

        response->ptr[size * nmemb] = '\0';
        response->len = size * nmemb;
        memcpy(response->ptr, ptr_in, size * nmemb);
    } else {
        // We have to sellotape the chunks together
        response->ptr = (char *) realloc(response->ptr,
                                         response->len + (size * nmemb) + 1);
        if (response->ptr == NULL) {
            lprintf(LOG_ERROR, "Malloc error\n");
            return 0;
        }

        response->ptr[response->len + (size * nmemb)] = '\0';
        memcpy(response->ptr + response->len, ptr_in, size * nmemb);
        response->len += size * nmemb;
    }

    return size * nmemb;
}

char *send_request(char * url, char *data, char *request, unsigned int nonce)
{
    CURL *curl = curl_easy_init();
    if(curl) {
        CURLcode res;

        curl_response response = {NULL, 0};

        struct curl_slist *list = NULL;
        list = curl_slist_append(list, "Accept: application/json");
        list = curl_slist_append(list, "Content-Type: application/json");

        char buffer[255];
        snprintf(buffer, sizeof(buffer), "Nonce: %u", nonce);
        list = curl_slist_append(list, buffer);

        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        // Set timeouts
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 2L);

        // Set url, user-agent and, headers
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, 0L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "f@antom");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data));
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request);

        // Set response write
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &response);

        res = curl_easy_perform(curl);

        int getSuccess = res == CURLE_OK && response.ptr != NULL;

        if (getSuccess) {
            return response.ptr;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(list);

        if (!getSuccess) {
            if (res != CURLE_OK) {
                lprintf(LOG_ERROR, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            } else {
                lprintf(LOG_ERROR, "No response\n");
            }
        }
    } else {
        lprintf(LOG_ERROR, "Curl init failed\n");
    }
    return NULL;
}

char *get_request(char *url)
{
    CURL *curl = curl_easy_init();
    if(curl) {
        CURLcode res;
        curl_response response = {NULL, 0};

        struct curl_slist *list = NULL;
        list = curl_slist_append(list, "Accept: application/json");
        list = curl_slist_append(list, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        // Set timeouts
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 2L);

        // Set url, user-agent and, headers
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, 0L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "f@ntom");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

        // Set response write
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &response);

        res = curl_easy_perform(curl);

        int getSuccess = res == CURLE_OK && response.ptr != NULL;

        if (getSuccess) {
            return response.ptr;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(list);

        if (!getSuccess) {
            if (res != CURLE_OK) {
                lprintf(LOG_ERROR, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            } else {
                lprintf(LOG_ERROR, "No response\n");
            }
        }
    } else {
        lprintf(LOG_ERROR, "Curl init failed\n");
    }
    return NULL;
}
