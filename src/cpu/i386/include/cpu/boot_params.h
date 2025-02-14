#ifndef BOOT_PARAMS_H
#define BOOT_PARAMS_H

#include "defs.h"

typedef struct {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t ext;
} __attribute__((packed)) upper_ram_t;

typedef struct {
    uint16_t    low_mem_size;
    uint16_t    mem_entries_count;
    upper_ram_t mem_entries[];
} __attribute__((packed)) boot_params_t;

boot_params_t * get_boot_params();

#endif // BOOT_PARAMS_H
