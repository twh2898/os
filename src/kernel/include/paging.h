#ifndef KERNEL_PAGING_H
#define KERNEL_PAGING_H

#include <stdint.h>

#include "addr.h"
#include "cpu/mmu.h"

/**
 * @brief Setup paging called by kernel main.
 */
void paging_init();

/**
 * @brief Map `paddr` to a temporary page and return the virtual address.
 *
 * @param paddr physical address to map
 * @return void* pointer to the virtual page address
 */
void * paging_temp_map(uint32_t paddr);

/**
 * @brief Free `paddr` previously mapped with `paging_temp_map`.
 *
 * The `paddr` argument should be the same as passed to `paging_temp_map`.
 *
 * @param paddr physical address to free
 */
void paging_temp_free(uint32_t paddr);

/**
 * @brief Get count of free pages for temporary mapping.
 *
 * @return size_t number of free temporary pages
 */
size_t paging_temp_available();

#endif // KERNEL_PAGING_H
