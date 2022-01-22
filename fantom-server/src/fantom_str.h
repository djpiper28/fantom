#pragma once
#include <stdlib.h>
#include "fantom_utils.h"

typedef struct fantom_str_t {
    char *ptr;
    size_t buffer_len;
} fantom_str_t;

fantom_status_t fantom_valid_str(fantom_str_t *);

// Init and free functions
fantom_status_t fantom_free_str(fantom_str_t *);
fantom_status_t fantom_init_str(fantom_str_t *);
fantom_status_t fantom_init_from_c_str(fantom_str_t *, const char *);

// String.h wrappers
fantom_status_t fantom_cpy_str(fantom_str_t *dest, fantom_str_t *src);
fantom_status_t fantom_concat_str(fantom_str_t *dest, fantom_str_t *src);
int fantom_cmp_str(fantom_str_t *a, fantom_str_t *b);

// Split and find
size_t fantom_find_str(fantom_str_t *input, fantom_str_t *substr);
fantom_status_t fantom_split_str(fantom_str_t *dest, fantom_str_t *src, size_t /*pos*/);

// Str util functions
size_t fantom_len_str(fantom_str_t *); // Check the length of the string
fantom_status_t fantom_is_good_str(fantom_str_t *); // Check that the string is usable

