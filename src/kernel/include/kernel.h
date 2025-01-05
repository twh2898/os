#ifndef KERNEL_H
#define KERNEL_H

#include <stddef.h>
#include <stdint.h>

#include "proc.h"

typedef struct _kernel {
    uint32_t   ram_table_addr;
    size_t     ram_table_count;
    uint32_t   cr3;
    uint32_t   kernel_table_vaddr; // First page table in any page dir maps kernel
    process_t  proc;
    proc_man_t pm;
} kernel_t;

/**
 * @brief Get a pointer to the kernel's page directory.
 *
 * This address is identity mapped and should be the same for virtual and
 * physical address spaces.
 *
 * @return mmu_dir_t* pointer to the kernel's page directory
 */
mmu_dir_t * get_kernel_dir();

/**
 * @brief Get a pointer to the virtual address of the first page table.
 *
 * The first page table is the kernel's memory space. This pointer is the
 * virtual address of the table within the kernel's memory space.
 *
 * @return mmu_table_t* pointer to the kernel's page table of any page directory
 */
mmu_table_t * get_kernel_table();

#endif // KERNEL_H
