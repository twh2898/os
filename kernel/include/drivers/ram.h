#ifndef RAM_H
#define RAM_H

#include <stdbool.h>
#include <stdint.h>

enum RAM_TYPE {
    RAM_TYPE_USABLE = 1,
    RAM_TYPE_RESERVED = 2,
    RAM_TYPE_ACPI_RECLAIMABLE = 3,
    RAM_TYPE_ACPI_NVS = 4,
    RAM_TYPE_BAD = 5,
};

void init_ram();

uint16_t ram_lower_size();

uint16_t ram_upper_count();

uint64_t ram_upper_start(uint16_t i);
uint64_t ram_upper_end(uint16_t i);
uint64_t ram_upper_size(uint16_t i);

bool ram_upper_usable(uint16_t i);
enum RAM_TYPE ram_upper_type(uint16_t i);

/*
 Bits 12 - 31 of memory address (ie. page aligned pointer)
 Same as found in Page Table entry.
 0 is error
 */
void * ram_page_alloc();

/*
 Bits 12 - 31 of memory address (ie. page aligned pointer)
 Same as found in Page Table entry.
 0 is error
 */
void ram_page_free(void * addr);

#endif // RAM_H
