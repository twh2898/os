#include "cpu/memory.h"

#include "libc/stdio.h"

#define LOWER_MEMORY_ADDR 0x7E00
#define UPPER_MEMORY_COUNT 0x7E02
#define UPPER_MEMORY_ADDR 0x7E04

typedef struct {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t ext;
} __attribute__((packed)) upper_mem_t;

static uint16_t lower_mem;
static uint16_t upper_mem_count;
static upper_mem_t * upper_mem;

static void sort_mem();

void init_memory() {
    lower_mem = *(uint16_t *)LOWER_MEMORY_ADDR;
    upper_mem_count = *(uint16_t *)UPPER_MEMORY_COUNT;
    upper_mem = (upper_mem_t *)UPPER_MEMORY_ADDR;
    sort_mem();
}

uint16_t memory_lower_size() {
    return lower_mem;
}

uint16_t memory_upper_count() {
    return upper_mem_count;
}

uint64_t memory_upper_start(uint16_t i) {
    return upper_mem[i].base_addr;
}

uint64_t memory_upper_end(uint16_t i) {
    return upper_mem[i].base_addr + upper_mem[i].length;
}

uint64_t memory_upper_size(uint16_t i) {
    return upper_mem[i].length;
}

bool memory_upper_usable(uint16_t i) {
    return upper_mem[i].type == MEMORY_TYPE_USABLE
           || upper_mem[i].type == MEMORY_TYPE_ACPI_RECLAIMABLE;
}

enum MEMORY_TYPE memory_upper_type(uint16_t i) {
    upper_mem_t * upper_mem = (upper_mem_t *)UPPER_MEMORY_ADDR;
    return upper_mem[i].type;
}

static void sort_mem() {
    upper_mem_t swap;

    // TODO handle overlap
    for (size_t i = 0; i < upper_mem_count - 1; i++) {
        uint64_t curr_start = upper_mem[i].base_addr;

        size_t next_i = i;
        uint64_t next_start = curr_start;

        for (size_t i = i; i < upper_mem_count; i++) {
            if (upper_mem[i].base_addr > next_start) {
                next_i = i;
                next_start = upper_mem[i].base_addr;
            }
        }

        if (next_i != i) {
            swap = upper_mem[i];
            upper_mem[i] = upper_mem[next_i];
            upper_mem[next_i] = upper_mem[i];
        }
    }
}
