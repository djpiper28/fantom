#pragma once
#include <stdio.h>
#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

char *read_file(FILE *input);
char *get_error_msg(char *msg);

