#pragma once

#define SALT_CHARS "abdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/*-+,.?/<>:@~;'#"
#define SALT_LENGTH 255
#define SHA512_DIGEST_STRING_LENGTH 128

void initSeed();
int hashPassword(char *password, char *salt, char **output, size_t *length);

