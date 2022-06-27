#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include "utils.h"
#include "logger.h"

char *read_file(FILE *input)
{
    if (input == NULL) {
        return NULL;
    }

    // Read form file
    size_t buffer_length = BUFFER_LENGTH;
    size_t buffer_pointer = 0;
    char *ptr = malloc(sizeof * ptr * buffer_length);

    // Read to buffer
    for (int c = EOF; (c = fgetc(input)) != EOF && ptr != NULL;) {
        // Grow buffer if needed
        if (buffer_pointer + 1 >= buffer_length) {
            buffer_length += BUFFER_LENGTH;
            char *next_ptr = realloc(ptr, buffer_length);

            if (next_ptr == NULL) {
                lprintf(LOG_ERROR, "Cannot assign memory\n");
                free(ptr);
                ptr = NULL;
            } else if (next_ptr != ptr) {
                ptr = next_ptr;
            }

        }

        // Check for errors in buffer growth
        if (ptr != NULL) {
            ptr[buffer_pointer++] = (char) c;
        }
    }
    // Grow buffer if needed
    if (buffer_pointer + 1 >= buffer_length) {
        buffer_length += 1;
        char *next_ptr = realloc(ptr, buffer_length);

        if (next_ptr == NULL) {
            lprintf(LOG_ERROR, "Cannot assign memory\n");
            free(ptr);
            ptr = NULL;
        } else if (next_ptr != ptr) {
            ptr = next_ptr;
        }

    }

    // Check for errors in buffer growth
    if (ptr != NULL) {
        ptr[buffer_pointer++] = 0;
    }

    return ptr;
}

char *get_error_msg(char *msg)
{
    json_t *obj = json_pack("{ss}", "error", msg);
    if (obj == NULL) {
        lprintf(LOG_ERROR, "Cannot encode error\n");
        return NULL;
    }

    char *ret = json_dumps(obj, JSON_ENCODE_ANY);
    json_decref(obj);

    return ret;
}

