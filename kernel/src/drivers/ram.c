#include "drivers/ram.h"

#include "libc/stdio.h"

#define LOWER_RAM_ADDR 0x7E00
#define UPPER_RAM_COUNT 0x7E02
#define UPPER_RAM_ADDR 0x7E04

typedef struct {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t ext;
} __attribute__((packed)) upper_ram_t;

static uint16_t lower_ram;
static uint16_t upper_ram_count;
static upper_ram_t * upper_ram;

static void sort_ram();

void init_ram() {
    lower_ram = *(uint16_t *)LOWER_RAM_ADDR;
    upper_ram_count = *(uint16_t *)UPPER_RAM_COUNT;
    upper_ram = (upper_ram_t *)UPPER_RAM_ADDR;
    sort_ram();
}

uint16_t ram_lower_size() {
    return lower_ram;
}

uint16_t ram_upper_count() {
    return upper_ram_count;
}

uint64_t ram_upper_start(uint16_t i) {
    return upper_ram[i].base_addr;
}

uint64_t ram_upper_end(uint16_t i) {
    return upper_ram[i].base_addr + upper_ram[i].length;
}

uint64_t ram_upper_size(uint16_t i) {
    return upper_ram[i].length;
}

bool ram_upper_usable(uint16_t i) {
    return upper_ram[i].type == RAM_TYPE_USABLE
           || upper_ram[i].type == RAM_TYPE_ACPI_RECLAIMABLE;
}

enum RAM_TYPE ram_upper_type(uint16_t i) {
    upper_ram_t * upper_ram = (upper_ram_t *)UPPER_RAM_ADDR;
    return upper_ram[i].type;
}

static void sort_ram() {
    upper_ram_t swap;

    // TODO handle overlap
    for (size_t i = 0; i < upper_ram_count - 1; i++) {
        uint64_t curr_start = upper_ram[i].base_addr;

        size_t next_i = i;
        uint64_t next_start = curr_start;

        for (size_t i = i; i < upper_ram_count; i++) {
            if (upper_ram[i].base_addr > next_start) {
                next_i = i;
                next_start = upper_ram[i].base_addr;
            }
        }

        if (next_i != i) {
            swap = upper_ram[i];
            upper_ram[i] = upper_ram[next_i];
            upper_ram[next_i] = upper_ram[i];
        }
    }
}
