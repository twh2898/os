#ifndef RAM_H
#define RAM_H

#include "defs.h"

enum RAM_TYPE {
    RAM_TYPE_USABLE = 1,
    RAM_TYPE_RESERVED = 2,
    RAM_TYPE_ACPI_RECLAIMABLE = 3,
    RAM_TYPE_ACPI_NVS = 4,
    RAM_TYPE_BAD = 5,
};

// THIS IS IN PHYSICAL ADDRESS SPACE NOT VIRTUAL
// Returns pointer to the pre-allocated pages (first page table, last page table, )
void * init_ram(void * ram_table, size_t * ram_table_count);

uint16_t ram_lower_size();

uint16_t ram_upper_count();

uint64_t ram_upper_start(uint16_t i);
uint64_t ram_upper_end(uint16_t i);
uint64_t ram_upper_size(uint16_t i);

bool ram_upper_usable(uint16_t i);
enum RAM_TYPE ram_upper_type(uint16_t i);
// END THIS IS IN PHYSICAL ADDRESS SPACE NOT VIRTUAL

uint32_t get_bitmask_addr(size_t i);

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
