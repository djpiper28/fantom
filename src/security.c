#include <stdio.h>
#include <iostream>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include "security.h"
#include "sys/time.h"

void initSeed()
{
    struct timeval seedTime;
    gettimeofday(&seedTime, NULL);
    srand(seedTime.tv_usec);
}

std::string getSalt()
{
    std::string ret = "";
    for (int i = 0; i < SALT_LENGTH; i++) {
        int random = abs(rand()) % (sizeof(SALT_CHARS) - 1);
        ret += SALT_CHARS[random];
    }

    return ret;
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

// WARNING: The hex must be lower case because the java program creates lower case hashes.
static char nibbleToHex(unsigned char nibble)
{
    if (nibble > 0x9) {
        return 'a' + nibble - 0xa;
    } else {
        return '0' + nibble;
    }
}

std::string hashPassword(std::string password, std::string salt, int *status)
{
    *status = 0;
    std::string ret = "";
    std::string toHash = password + salt;

    // Ask openssl to nicely hash my salted password
    unsigned int length;
    unsigned char *digest = NULL;

    int s = digest_message((unsigned char*) toHash.c_str(),
                           toHash.size(),
                           &digest,
                           &length);

    if (digest == NULL || s != 1) {
        std::cerr << "Failed to hash password" << std::endl;
        return "";
    }

    // Binary to hex
    for (unsigned int i = 0; i < length; i ++) {
        ret += nibbleToHex(digest[i] >> 4);
        ret += nibbleToHex(digest[i] & 0xF);
    }

    free(digest);

    *status = 1;
    return ret;
}

