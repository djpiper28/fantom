#pragma once
#include <pthread.h>
#include "fantom_utils.h"
#include "config.h"

#define SALT_CHARS "abdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/*-+,.?/<>:@~;'#"
#define SALT_LENGTH 256
#define SHA512_DIGEST_STRING_LENGTH 128

#define NONCE_MULT 30
#define NONCE_BASE 4096
#define NONCE_MAP_SIZE (NONCE_BASE * NONCE_MULT)
#define NONCE_MAX_COUNT NONCE_BASE
#define NONCE_TIMEOUT_S 10
#define NONCE_POLL_TIME_MS 10
#define NONCE_MAP_GRAVE_MARKER 1

#define FANTOM_AUD "F@ntom"
#define FANTOM_TIME_TOLERANCE 10

typedef struct fantom_claims_t {
    int uid;
    long expiry_time;
    long issue_time;
} fantom_claims_t;

typedef struct fantom_nonce_t {
    long issue_time;
    unsigned int nonce;
} fantom_nonce_t;

typedef struct fantom_nonce_manager_t {
    // Internal State
    pthread_t poll_thread;
    volatile int poll_thread_running;
    pthread_mutex_t lock_var;

    // Logical State
    int front_ptr, back_ptr, nonces;
    fantom_nonce_t *nonce_queue;
    unsigned int *nonce_map;
} fantom_nonce_manager_t;

fantom_status_t init_nonce_manager(fantom_nonce_manager_t *nonce_mgr);
void free_nonce_manager(fantom_nonce_manager_t *nonce_mgr);
fantom_status_t get_nonce(fantom_nonce_manager_t *nonce_mgr, unsigned int *ret);
fantom_status_t use_nonce(fantom_nonce_manager_t *mgr, unsigned int nonce);

// Internal logical functions exposed for testing.
int get_new_nonce_map_index(fantom_nonce_manager_t *mgr, unsigned int nonce); // not thread safe
int get_nonce_map_index(fantom_nonce_manager_t *mgr, unsigned int nonce);     // not thread safe

// Password stuff
void init_seed();
char *get_salt();
char *hash_password(char *password, char *salt);

// JWT stuff
char *issue_token(int uid, char *name, char *jwt_secret, fantom_config_t *config);
fantom_status_t use_token(char *token, char *jwt_secret, fantom_config_t *config);

