#include <stdio.h>
#include <time.h>
#include "./ram.h"

#define MEM_INFO "/proc/meminfo"
#define LINE_MEM_TOTAL 0
#define LINE_MEM_FREE 1
#define LINE_SWAP_TOTAL 14
#define LINE_SWAP_FREE 15

long read_meminfo_line(char *line)
{
    long ret = 0;
    for (int i = 0; line[i] != 0 && line[0] != '\n'; i++) {
        if (line[i] >= '0' && line[i] <= '9') {
            ret *= 10;
            ret += line[i] - '0';
        }
    }

    return ret;
}

fantom_status_t read_mem_info(fantom_ram_record_t *ret)
{
    FILE *f = fopen(MEM_INFO, "r");
    if (f == NULL) {
        return FANTOM_FAIL;
    }

    char buffer[256];
    ret->time = time(NULL);

    for (int i = 0; fgets(buffer, sizeof(buffer), f) != NULL; i++) {
        long val = read_meminfo_line(buffer);
        switch(i) {
        case LINE_MEM_TOTAL:
            ret->ram_total = val;
            break;
        case LINE_MEM_FREE:
            ret->ram_free = val;
            break;
        case LINE_SWAP_TOTAL:
            ret->swap_total = val;
            break;
        case LINE_SWAP_FREE:
            ret->swap_free = val;
            break;
        }
    }

    fclose(f);

    return FANTOM_SUCCESS;
}

