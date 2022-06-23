#pragma once
#include "../fantom_utils.h"

typedef struct fantom_cpu_record_t {
    double val; // sum of CPU usage for each processor
    int cores;
    long time;
} fantom_cpu_record_t;

fantom_status_t read_cpu_info(fantom_cpu_record_t *ret);

