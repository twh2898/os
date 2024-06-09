#include "cpu/mmu.h"

#include <stddef.h>

#include "drivers/page.h"
#include "kernel.h"
#include "libc/string.h"

#define PAGE_SIZE 0x1000 // 4k (>> 12)
#define PAGE_ALIGNED_DOWN(PTR) ((PTR) & 0xfffff000)
#define PAGE_ALIGNED_UP(PTR) ((PAGE_ALIGNED_DOWN(PTR)) + PAGE_SIZE)
#define PAGE_ALIGNED(PTR) (((PTR) & 0xfff) ? PAGE_ALIGNED_UP(PTR) : (PTR))

// mmu_page_dir_t * identity_dir = 0x1000;

mmu_page_dir_t * mmu_dir_create() {
    mmu_page_dir_t * dir = page_alloc();
    if (dir) {
        memset(dir, 0, sizeof(mmu_page_dir_t));
    }
    return dir;
}

void mmu_dir_free(mmu_page_dir_t * dir) {
    if (dir) {
        for (size_t i = 0; i < PAGE_DIR_SIZE; i++) {
            if (dir->entries[i] & MMU_PAGE_DIR_FLAGS_PRESENT) {
                uint32_t table_addr = dir->entries[i] & 0xfffff000;
                mmu_table_free((mmu_page_table_t *)table_addr);
            }
        }
    }
    page_free(dir);
}

mmu_page_table_t * mmu_table_create() {
    mmu_page_table_t * table = page_alloc();
    if (table) {
        memset(table, 0, sizeof(mmu_page_table_t));
    }
    return table;
}

void mmu_table_free(mmu_page_table_t * table) {
    if (table) {
        for (size_t i = 0; i < PAGE_TABLE_SIZE; i++) {
            if (table->entries[i] & MMU_PAGE_TABLE_FLAGS_PRESENT) {
                uint32_t page_addr = table->entries[i] & 0xfffff000;
                page_free((void *)page_addr);
            }
        }
    }
    page_free(table);
}

void mmu_dir_set_table(mmu_page_dir_t * dir, size_t i, mmu_page_table_t * table) {
    if (i >= PAGE_DIR_SIZE)
        return;

    mmu_page_entry_t entry = dir->entries[i];
    entry &= 0xfff;
    entry |= ((uint32_t)table) & 0xfffff000;

    dir->entries[i] = entry;
}

void mmu_dir_set_flags(mmu_page_dir_t * dir, size_t i, enum MMU_PAGE_DIR_FLAGS flags) {
    if (i >= PAGE_DIR_SIZE)
        return;

    mmu_page_entry_t entry = dir->entries[i];
    entry &= 0xfffff000;
    entry |= flags & 0xfff;

    dir->entries[i] = entry;
}

void mmu_table_set_addr(mmu_page_table_t * table, size_t i, uint32_t page_addr) {
    if (i >= PAGE_TABLE_SIZE)
        return;

    mmu_page_entry_t entry = table->entries[i];
    entry &= 0xfff;
    entry |= page_addr & 0xfffff000;

    table->entries[i] = entry;
}

void mmu_table_set_flags(mmu_page_table_t * table,
                         size_t i,
                         enum MMU_PAGE_TABLE_FLAGS flags) {
    if (i >= PAGE_TABLE_SIZE)
        return;

    mmu_page_entry_t entry = table->entries[i];
    entry &= 0xfffff000;
    entry |= flags & 0xfff;

    table->entries[i] = entry;
}

// void * mmu_phys_addr(void * virt_addr) {
//     unsigned long pdindex = (unsigned long)virt_addr >> 22;

//     // TODO THIS NEEDS TO BE WHERE THE PAGE DIR GETS MAPPED TO IN PAGING
//     unsigned long * pd = (unsigned long *)0xFFFFF000;
//     // TODO Here you need to check whether the PD entry is present.

//     // TODO THIS NEEDS TO BE WHERE THE PAGE TABLE GETS MAPPED TO IN PAGING
//     unsigned long * pt = ((unsigned long *)0xFFC00000) + (0x400 * pdindex);
//     // TODO Here you need to check whether the PT entry is present.

//     return (void *)((pt[ptindex] & ~0xFFF) + ((unsigned long)virt_addr &
//     0xFFF));
// }

// void mmu_map_page(mmu_page_dir_t * dir,
//                   void * phys_addr,
//                   void * virtual_addr,
//                   unsigned int flags) {
//     if (mmu_paging_enabled())
//         KERNEL_PANIC("Tried to map page to virtual address with paging
//         enabled");

//     // Make sure that both addresses are page-aligned.

//     unsigned long pdindex = (unsigned long)virtual_addr >> 22;
//     unsigned long ptindex = (unsigned long)virtual_addr >> 12 & 0x03FF;

//     // TODO THIS NEEDS TO BE WHERE THE PAGE DIR GETS MAPPED TO IN PAGING
//     unsigned long * pd = (unsigned long *)dir;
//     // Here you need to check whether the PD entry is present.
//     // When it is not present, you need to create a new empty PT and
//     // adjust the PDE accordingly.

//     // TODO THIS NEEDS TO BE WHERE THE PAGE TABLE GETS MAPPED TO IN PAGING
//     unsigned long * pt = ((unsigned long *)0xFFC00000) + (0x400 * pdindex);
//     // Here you need to check whether the PT entry is present.
//     // When it is, then there is already a mapping present. What do you do now?

//     pt[ptindex] = ((unsigned long)phys_addr) | (flags & 0xFFF) | 0x01; //
//     Present

//     // Now you need to flush the entry in the TLB
//     // or you might not notice the change.
// }
