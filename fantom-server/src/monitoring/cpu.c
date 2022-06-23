#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/sysinfo.h>
#include "./cpu.h"

#define PROC_STAT "/proc/stat"

static double read_next_number(char *buffer, int *i)
{
    long ret = 0;
    for (; buffer[*i] >= '0' && buffer[*i] <= '9' ; *i += 1) {
        ret *= 10;
        ret += buffer[*i] - '0';
    }
    for(; buffer[*i] < '0' || buffer[*i] > '9'; *i += 1);

    return (double) ret;
}

static double get_load(char *buffer)
{
    int i = 0;
    for(; buffer[i] < '0' || buffer[i] > '9'; i++);

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
    FILE *f = fopen(PROC_STAT, "r");
    if (f == NULL) {
        return FANTOM_FAIL;
    }

    ret->time = time(NULL);
    ret->cores = get_nprocs();
    ret->val = 0;

    for (int i = 0; i < ret->cores; i++) {
        char buffer[256];
        char *r = fgets(buffer, sizeof(buffer), f);
        if (r == NULL) {
            break;
        }

        ret->val += get_load(buffer);
    }

    fclose(f);
    return FANTOM_SUCCESS;
}

fantom_status_t read_cpu_cores_info(fantom_cpu_cores_record_t *ret)
{
    FILE *f = fopen(PROC_STAT, "r");
    if (f == NULL) {
        return FANTOM_FAIL;
    }

    ret->time = time(NULL);
    ret->cores = get_nprocs();
    ret->vals = malloc(sizeof(ret->vals) * ret->cores);
    if (ret->vals == NULL) {
        free(ret->vals);
        fclose(f);
        return FANTOM_FAIL;
    }

    for (int i = 0; i < ret->cores; i++) {
        char buffer[256];
        char *r = fgets(buffer, sizeof(buffer), f);
        if (r == NULL) {
            break;
        }

        ret->vals[i] = get_load(buffer);
    }

    fclose(f);
    return FANTOM_SUCCESS;
}

void free_cpu_cores_record(fantom_cpu_cores_record_t *rec)
{
    if (rec->vals != NULL) {
        free(rec->vals);
    }
}

