#ifndef KERNEL_PAGING_H
#define KERNEL_PAGING_H

#include <stdint.h>

#include "addr.h"
#include "cpu/mmu.h"

// Create task page dir

void paging_init();

void * paging_temp_map(uint32_t paddr);
void   paging_temp_free(uint32_t paddr);
size_t paging_temp_available();

#endif // KERNEL_PAGING_H
