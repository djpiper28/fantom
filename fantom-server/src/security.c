#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <openssl/evp.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include "security.h"
#include "logger.h"

void init_seed()
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

// WARNING: The hex must be lower case
static char nibble_to_hex(unsigned char nibble)
{
    if (nibble > 0x9) {
    				return 'a' + nibble - 0xa;
    } else {
    				return '0' + nibble;
    }
}

char *get_salt()
{
    char *buffer = malloc(sizeof * buffer * SALT_LENGTH);
    if (buffer == NULL) {
        lprintf(LOG_ERROR, "Could not assign memory\n");
        return NULL;
		}
 
    for (int i = 0; i < SALT_LENGTH; i++) {
        int random = abs(rand()) % (sizeof(SALT_CHARS) - 1);
        buffer[i] = SALT_CHARS[random];
    }

    return buffer;
}

char *hash_password(char *password, char *salt)
{
    size_t pass_len = strlen(password);
    size_t salt_len = strlen(salt);
    size_t buffer_length = salt_len + pass_len;
    char *buffer = malloc(sizeof * buffer * (buffer_length + 1));
    if (buffer == NULL) {
        lprintf(LOG_ERROR, "Could not assign memory\n");
        return NULL;
    }

    strcpy(buffer, password);
    strcpy(buffer + pass_len, salt);
    buffer[buffer_length] = 0;

    // Ask openssl to nicely hash my salted password
    unsigned int l;
    unsigned char *digest = NULL;

    int s = digest_message((unsigned char*) buffer,
                           buffer_length,
                           &digest,
                           &l);

    if (digest == NULL || s != 1) {
        lprintf(LOG_ERROR, "Failed to hash password\n");
        return NULL;
    }

    if (l * 2 != SHA512_DIGEST_STRING_LENGTH) {
    	  lprintf(LOG_ERROR, "Hash length is wrong\n");
    	  return NULL;
    }

    char *hex_hash = malloc(sizeof * hex_hash * (SHA512_DIGEST_STRING_LENGTH + 1));
    if (hex_hash == NULL) {
        lprintf(LOG_ERROR, "Could not assign memory\n");
    	  free(digest);
    	  return NULL;
    }
    
    for (size_t i = 0; i < l && i < SHA512_DIGEST_STRING_LENGTH / 2; i++) {
    	  hex_hash[2 * i] = nibble_to_hex((digest[i] >> 4) & 0xF);
    	  hex_hash[2 * i + 1] = nibble_to_hex(digest[i] & 0xF);
    }
    hex_hash[SHA512_DIGEST_STRING_LENGTH] = 0;
    free(digest);

		return hex_hash;
}

