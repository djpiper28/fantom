#pragma once

#define SALT_CHARS "abdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/*-+,.?/<>:@~;'#"
#define SALT_LENGTH 256
#define SHA512_DIGEST_STRING_LENGTH 128

void init_seed();
char *get_salt();
char *hash_password(char *password, char *salt);

