#pragma once
#include "../fantom_utils.h"

typedef struct fantom_ram_record_t {
    long ram_free, swap_free;
    long ram_total, swap_total;
    long time;
} fantom_ram_record_t;

fantom_status_t read_mem_info(fantom_ram_record_t *ret);

