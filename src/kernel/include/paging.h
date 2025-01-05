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

/**
 * @brief Map a virtual address to a physical address. The addresses must be
 * page aligned.
 *
 * @param vaddr virtual address
 * @param paddr physical address
 * @param flags page table entry flags
 * @return int 0 for success
 */
int paging_map(uint32_t vaddr, uint32_t paddr, enum MMU_TABLE_FLAG flags);

/**
 * @brief Identity map a range of pages.
 *
 * @param start first page index
 * @param end last page index
 * @return int 0 for success
 */
int paging_id_map_range(size_t start, size_t end);

/**
 * @brief Identity map a single page.
 *
 * @param page page index
 * @return int 0 for success
 */
int paging_id_map_page(size_t page);

#endif // KERNEL_PAGING_H
