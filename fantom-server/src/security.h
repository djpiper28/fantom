#pragma once
#include <string>

#define SALT_CHARS "abdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/*-+,.?/<>:@~;'#"
#define SALT_LENGTH 255
#define SHA512_DIGEST_STRING_LENGTH 128

void initSeed();
std::string getSalt();
std::string hashPassword(std::string password, std::string salt, int *status);

