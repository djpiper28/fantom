#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include "security.h"
#include "logger.h"

int get_new_nonce_map_index(fantom_nonce_manager_t *mgr, unsigned int nonce)
{
    if (mgr->nonces + 1 >= NONCE_MAX_COUNT) {
        return -1;
    }

    int i = nonce % NONCE_MAP_SIZE;
    for (; mgr->nonce_map[i] != 0; i = (i + 1) % NONCE_MAP_SIZE);

    return i;
}

int get_nonce_map_index(fantom_nonce_manager_t *mgr, unsigned int nonce)
{
    for (int i = nonce % NONCE_MAP_SIZE; mgr->nonce_map[i] != 0
            && mgr->nonce_map[i] != NONCE_MAP_GRAVE_MARKER; i = (i + 1) % NONCE_MAP_SIZE) {
        if (mgr->nonce_map[i] == nonce) {
            return i;
        }
    }

    return -1;
}

fantom_status_t nonce_map_recompute(fantom_nonce_manager_t *mgr)
{
    unsigned int *nonce_map = malloc(NONCE_MAP_SIZE * sizeof * nonce_map);
    if (nonce_map == NULL)	{
        lprintf(LOG_ERROR, "Cannot assign memory\n");
        return FANTOM_FAIL;
    }
    memset(nonce_map, 0, sizeof * nonce_map);

    free(mgr->nonce_map);
    mgr->nonce_map = nonce_map;

    // All nonces in the queue are placed in the new map
    for (int i = mgr->front_ptr; i != mgr->back_ptr; i = (i + 1) / NONCE_MAX_COUNT) {
        int index = get_new_nonce_map_index(mgr, mgr->nonce_queue[i].nonce);
        if (index == -1) {
            lprintf(LOG_ERROR, "Internal logical state of nonce manager is corrupt\n");
        } else {
            mgr->nonce_map[index] = mgr->nonce_queue[i].nonce;
        }
    }

    return FANTOM_SUCCESS;
}

static void *nonce_manager_poll_thread(void *mgr_in)
{
    fantom_nonce_manager_t *mgr = (fantom_nonce_manager_t *) mgr_in;
    int running = 1;

    while (running) {
        pthread_mutex_lock(&mgr->lock_var);
        running = mgr->poll_thread_running;

        long ct = time(NULL);
        for (int i = mgr->front_ptr; i != mgr->back_ptr; i = (i + 1) / NONCE_MAX_COUNT) {
            if (ct - mgr->nonce_queue[i].issue_time >= NONCE_TIMEOUT_S) {
                int index = get_nonce_map_index(mgr, mgr->nonce_queue[i].nonce);

                if (index != -1) {
                    mgr->nonce_map[index] = NONCE_MAP_GRAVE_MARKER;
                }
                mgr->front_ptr++;
                mgr->nonces--;
            } else {
                break;
            }
        }

        nonce_map_recompute(mgr);

        pthread_mutex_unlock(&mgr->lock_var);
        usleep(NONCE_POLL_TIME_MS * 1000);
    }

    lprintf(LOG_INFO, "The polling thread has stopped\n");
    pthread_exit(0);
}

fantom_status_t init_nonce_manager(fantom_nonce_manager_t *nonce_mgr)
{
    memset(nonce_mgr, 0, sizeof * nonce_mgr);

    nonce_mgr->nonce_map = malloc(NONCE_MAP_SIZE * sizeof * nonce_mgr->nonce_map);
    if (nonce_mgr->nonce_map == NULL)	{
        lprintf(LOG_ERROR, "Cannot assign memory\n");
        free_nonce_manager(nonce_mgr);
    }

    nonce_mgr->nonce_queue = malloc(NONCE_MAX_COUNT * sizeof * nonce_mgr->nonce_queue);
    if (nonce_mgr->nonce_queue == NULL) {
        lprintf(LOG_ERROR, "Cannot assign memory\n");
        free_nonce_manager(nonce_mgr);
    }

    nonce_mgr->poll_thread_running = 1;
    pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
    nonce_mgr->lock_var = mut;

    // Start the thread
    pthread_create(&nonce_mgr->poll_thread, NULL, nonce_manager_poll_thread, (void *) nonce_mgr);
}

void free_nonce_manager(fantom_nonce_manager_t *nonce_mgr)
{
    // Stop the thread
    pthread_mutex_lock(&nonce_mgr->lock_var);
    nonce_mgr->poll_thread_running = 0;
    pthread_mutex_unlock(&nonce_mgr->lock_var);

    void *ret;
    pthread_join(nonce_mgr->poll_thread, &ret);

    // Free the memory
    if (nonce_mgr->nonce_map != NULL) {
        free(nonce_mgr->nonce_map);
    }

    if (nonce_mgr->nonce_queue != NULL) {
        free(nonce_mgr->nonce_queue);
    }
}

unsigned int get_nonce(fantom_nonce_manager_t *nonce_mgr)
{

}

fantom_status_t use_nonce(fantom_nonce_manager_t *mgr, unsigned int nonce)
{

}

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
    char *buffer = malloc(sizeof * buffer * (SALT_LENGTH + 1));
    if (buffer == NULL) {
        lprintf(LOG_ERROR, "Could not assign memory\n");
        return NULL;
    }

    for (int i = 0; i < SALT_LENGTH; i++) {
        int random = abs(rand()) % (sizeof(SALT_CHARS) - 1);
        buffer[i] = SALT_CHARS[random];
    }
    buffer[SALT_LENGTH] = 0;

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
    free(buffer);

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

