#include <stdio.h>
#include <string.h>
#include "fantom_str.h"
#include "fantom_utils.h"

fantom_status fantom_valid_str(fantom_str *str)
{
    if (str == NULL) {
        return FANTOM_FAIL;
    }
    
    if (str->ptr == NULL) {
        return FANTOM_FAIL;
    }
    
    return FANTOM_SUCCESS;
}


fantom_status fantom_free_str(fantom_str *str)
{
    if (str->ptr != NULL) {
        free(str->ptr);
        str->ptr = NULL;
        str->buffer_len = 0;
        
        return FANTOM_SUCCESS;
    } else {
        fprintf(stderr, "[ERROR]: Cannot free fantom string: ptr is NULL.\n");
        return FANTOM_FAIL;
    }
}

fantom_status fantom_init_str(fantom_str *str)
{
    char *ptr = malloc(sizeof * ptr);
    if (ptr == NULL) {
        fprintf(stderr, "[ERROR]: Cannot init fantom string: ptr is NULL after malloc.\n");
        return FANTOM_FAIL;
    } else {
        ptr[0] = 0; // Null terminator as this is an empty string
        str->ptr = ptr;
        str->buffer_len = 0;
        return FANTOM_SUCCESS;
    }
}

fantom_status fantom_init_from_c_str(fantom_str *str, const char *c_str)
{
    size_t length = strlen(c_str);
    char *ptr = malloc(sizeof * ptr * (length + 1));
    
    if (ptr == NULL) {
        fprintf(stderr, "[ERROR]: Cannot init fantom string from c string: ptr is NULL after malloc.\n");
        return FANTOM_FAIL;
    } else {
        strcpy(ptr, c_str);
        ptr[length] = 0;
        str->ptr = ptr;
        str->buffer_len = length;
        return FANTOM_SUCCESS;
    }
}

fantom_status fantom_cpy_str(fantom_str *dest, fantom_str *src)
{
    if (fantom_valid_str(dest) || fantom_valid_str(src)) {
        return FANTOM_FAIL;
    }
    
    size_t length = src->buffer_len;
    char *ptr = malloc(sizeof * ptr * (length + 1));
    
    if (ptr == NULL) {
        fprintf(stderr, "[ERROR]: Cannot copy fantom string from c string: ptr is NULL after malloc.\n");
        return FANTOM_FAIL;
    } else {
        strcpy(ptr, src->ptr);
        ptr[length] = 0;
        
        fantom_free_str(dest);
        dest->ptr = ptr;
        dest->buffer_len = length;
        return FANTOM_SUCCESS;
    }
}

fantom_status fantom_concat_str(fantom_str *dest, fantom_str *src)
{
    if (fantom_valid_str(dest) || fantom_valid_str(src)) {
        return FANTOM_FAIL;
    }
    
    size_t length = dest->buffer_len + src->buffer_len;
    char *ptr = malloc(sizeof * ptr * (length + 1));
    
    if (ptr == NULL) {
        fprintf(stderr, "[ERROR]: Cannot concat fantom string from c string: ptr is NULL after malloc.\n");
        return FANTOM_FAIL;
    } else {
        strcpy(ptr, dest->ptr);
        strcpy(ptr + dest->buffer_len, src->ptr);
        ptr[length] = 0;
        dest->ptr = ptr;
        dest->buffer_len = length;
        return FANTOM_SUCCESS;
    }
}

int fantom_cmp_str(fantom_str *a, fantom_str *b)
{
    if (fantom_valid_str(a) || fantom_valid_str(b)) {
        return FANTOM_FAIL;
    }
    
    return strcmp(a->ptr, b->ptr);
}
