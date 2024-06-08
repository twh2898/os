#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>

void init_pages();

/*
 Bits 12 - 31 of memory address (ie. page aligned pointer)
 Same as found in Page Table entry.
 0 is error
 */
uint32_t page_alloc();

/*
 Bits 12 - 31 of memory address (ie. page aligned pointer)
 Same as found in Page Table entry.
 0 is error
 */
void page_free(uint32_t addr);

#endif // PAGE_H
