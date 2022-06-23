#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/sysinfo.h>
#include "./cpu.h"

#define LOAD_AVG "/proc/stat"

static double read_next_number(char *buffer, int *i)
{
    long ret = 0;
    for (; buffer[*i] != ' ' && buffer[*i] != '\n' && buffer[*i] > 0; *i += 1) {
        if (buffer[*i] >= '0' && buffer[*i] <= '9') {
            ret *= 10;
            ret += buffer[*i];
        }
    }

    return (double) ret;
}

static double get_load(char *buffer)
{
    // cpu4 127576 69224 82851 1411192 484 0 712 0 0 0
    int i = 0;
    for(; buffer[i] != ' '; i++);

    long user = read_next_number(buffer, &i),
         nice = read_next_number(buffer, &i),
         system = read_next_number(buffer, &i),
         idle = read_next_number(buffer, &i),
         iowait = read_next_number(buffer, &i),
         irq = read_next_number(buffer, &i),
         softirq = read_next_number(buffer, &i);

    return (idle * 100) / (user + nice + system + idle + iowait + irq + softirq);
}

fantom_status_t read_cpu_info(fantom_cpu_record_t *ret)
{
    FILE *f = fopen(LOAD_AVG, "r");
    if (f == NULL) {
        return FANTOM_FAIL;
    }

    ret->time = time(NULL);
    ret->cores = get_nprocs();
    ret->val = 0;

    for (int i = 0; i < ret->cores; i++) {
        char buffer[256];
        char *r = fgets(buffer, sizeof(buffer), f);
        ret->val += get_load(buffer);
    }

    fclose(f);
    return FANTOM_SUCCESS;
}

