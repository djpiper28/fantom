#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <openssl/evp.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include "security.h"
#include "logger.h"

void initSeed()
{
    struct timeval seedTime;
    gettimeofday(&seedTime, NULL);
    srand(seedTime.tv_usec);
}

static int digest_message(const unsigned char *message,
                          size_t message_len,
                          unsigned char **digest,
                          unsigned int *digest_len)
{
    EVP_MD_CTX *mdctx;

    if((mdctx = EVP_MD_CTX_new()) == NULL) {
        return 0;
    }

    if(1 != EVP_DigestInit_ex(mdctx, EVP_sha512(), NULL)) {
        return 0;
    }

    if(1 != EVP_DigestUpdate(mdctx, message, message_len)) {
        return 0;
    }

    if((*digest = (unsigned char *) malloc(EVP_MD_size(EVP_sha512()))) == NULL) {
        return 0;
    }

    if(1 != EVP_DigestFinal_ex(mdctx, *digest, digest_len)) {
        return 0;
    }

    EVP_MD_CTX_free(mdctx);
    return 1;
}

int hashPassword(char *password, char *salt, char **output, size_t *length)
{
    size_t pass_len = strlen(password);
    size_t buffer_length = SALT_LENGTH + pass_len;
    char *buffer = malloc(sizeof * buffer * buffer_length);
    if (buffer == NULL) {
        return 0;
    }

    strcpy(buffer, password);
    for (int i = 0; i < SALT_LENGTH; i++) {
        int random = abs(rand()) % (sizeof(SALT_CHARS) - 1);
        buffer[i + pass_len] = SALT_CHARS[random];
    }

    // Ask openssl to nicely hash my salted password
    unsigned int l;
    unsigned char *digest = NULL;

    int s = digest_message((unsigned char*) buffer,
                           buffer_length,
                           &digest,
                           &l);

    if (digest == NULL || s != 1) {
        lprintf(LOG_ERROR, "Failed to hash password\n");
        return 0;
    }

    *output = (char *) digest;
    *length = (size_t) l;
    return 1;
}

