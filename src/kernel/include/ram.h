#ifndef RAM_H
#define RAM_H

#include "defs.h"

enum RAM_TYPE {
    RAM_TYPE_USABLE           = 1,
    RAM_TYPE_RESERVED         = 2,
    RAM_TYPE_ACPI_RECLAIMABLE = 3,
    RAM_TYPE_ACPI_NVS         = 4,
    RAM_TYPE_BAD              = 5,
};

// THIS IS IN PHYSICAL ADDRESS SPACE NOT VIRTUAL
// Returns pointer to the pre-allocated pages (first page table, last page table, )
void ram_init(void * ram_table, size_t * ram_table_count);

uint16_t ram_lower_size();

uint16_t ram_upper_count();

uint64_t ram_upper_start(uint16_t i);
uint64_t ram_upper_end(uint16_t i);
uint64_t ram_upper_size(uint16_t i);

bool          ram_upper_usable(uint16_t i);
enum RAM_TYPE ram_upper_type(uint16_t i);
// END THIS IS IN PHYSICAL ADDRESS SPACE NOT VIRTUAL

uint32_t ram_bitmask_paddr(size_t region_index);
uint32_t ram_bitmask_vaddr(size_t region_index);

/*
 Bits 12 - 31 of memory address (ie. page aligned pointer)
 Same as found in Page Table entry.
 0 is error
 */
uint32_t ram_page_alloc();

// Allocate in physical address space (before paging)
uint32_t ram_page_palloc();

/*
 Bits 12 - 31 of memory address (ie. page aligned pointer)
 Same as found in Page Table entry.
 0 is error
 */
void ram_page_free(uint32_t addr);

#endif // RAM_H
