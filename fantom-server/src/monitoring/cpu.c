#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/sysinfo.h>
#include "./cpu.h"

#define LOAD_AVG "/proc/loadavg"

fantom_status_t read_cpu_info(fantom_cpu_record_t *ret)
{
    FILE *f = fopen(LOAD_AVG, "r");
    if (f == NULL) {
        return FANTOM_FAIL;
    }

    ret->time = time(NULL);
    int c = 0, i = 0;
    char buffer[25];
    for (; c = fgetc(f), c != ' ' && c != EOF && (size_t) i < sizeof(buffer);) {
        if ((c >= '0' && c <= '9') || c == '.') {
            buffer[i] = c;
            i++;
        }
    }
    buffer[i] = 0;

    ret->cores = get_nprocs();
    ret->val = atof(buffer);

    fclose(f);
    return FANTOM_SUCCESS;
}

