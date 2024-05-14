#ifndef CPU_MEMORY_H
#define CPU_MEMORY_H

#include <stdbool.h>
#include <stdint.h>

enum MEMORY_TYPE {
    MEMORY_TYPE_USABLE = 1,
    MEMORY_TYPE_RESERVED = 2,
    MEMORY_TYPE_ACPI_RECLAIMABLE = 3,
    MEMORY_TYPE_ACPI_NVS = 4,
    MEMORY_TYPE_BAD = 5,
};

void init_memory();
void print_memory();

uint16_t memory_lower_size();

uint16_t memory_upper_count();

uint64_t memory_upper_start(uint16_t i);
uint64_t memory_upper_end(uint16_t i);
uint64_t memory_upper_size(uint16_t i);

bool memory_upper_usable(uint16_t i);
enum MEMORY_TYPE memory_upper_type(uint16_t i);

#endif // CPU_MEMORY_H
