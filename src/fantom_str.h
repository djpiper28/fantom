#pragma once
#include <stdlib.h>
#include "fantom_utils.h"

typedef struct fantom_str {
    char *ptr;
    size_t buffer_len;
} fantom_str;

fantom_status fantom_valid_str(fantom_str *);

// Init and free functions
fantom_status fantom_free_str(fantom_str *);
fantom_status fantom_init_str(fantom_str *);
fantom_status fantom_init_from_c_str(fantom_str *, const char *);

// String.h wrappers
fantom_status fantom_cpy_str(fantom_str */*dest*/, fantom_str */*src*/);
fantom_status fantom_concat_str(fantom_str */*dest*/, fantom_str */*src*/);
int fantom_cmp_str(fantom_str *, fantom_str *);

// Split and find
size_t fantom_find_str(fantom_str */*input*/, fantom_str */*substr*/);
fantom_status fantom_split_str(fantom_str */*dest*/, fantom_str */*src*/, size_t /*pos*/);

// Str util functions
size_t fantom_len_str(fantom_str *); // Check the length of the string
fantom_status fantom_is_good_str(fantom_str *); // Check that the string is usable
