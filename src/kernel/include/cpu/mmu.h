#ifndef MMU_H
#define MMU_H

#include "defs.h"

#define PAGE_DIR_SIZE   1024
#define PAGE_TABLE_SIZE 1024

#define MMU_DIR_RW   (MMU_PAGE_DIR_FLAG_PRESENT | MMU_PAGE_DIR_FLAG_READ_WRITE)
#define MMU_TABLE_RW (MMU_PAGE_TABLE_FLAG_PRESENT | MMU_PAGE_TABLE_FLAG_READ_WRITE)

enum MMU_PAGE_DIR_FLAG {
    MMU_PAGE_DIR_FLAG_PRESENT = 0x1,
    MMU_PAGE_DIR_FLAG_READ_WRITE = 0x2,
    MMU_PAGE_DIR_FLAG_USER_SUPERVISOR = 0x4,
    MMU_PAGE_DIR_FLAG_WRITE_THROUGH = 0x8,
    MMU_PAGE_DIR_FLAG_CACHE_DISABLE = 0x10,
    MMU_PAGE_DIR_FLAG_ACCESSED = 0x20,
    MMU_PAGE_DIR_FLAG_PAGE_SIZE = 0x80,
};

enum MMU_PAGE_TABLE_FLAG {
    MMU_PAGE_TABLE_FLAG_PRESENT = 0x1,
    MMU_PAGE_TABLE_FLAG_READ_WRITE = 0x2,
    MMU_PAGE_TABLE_FLAG_USER_SUPERVISOR = 0x4,
    MMU_PAGE_TABLE_FLAG_WRITE_THROUGH = 0x8,
    MMU_PAGE_TABLE_FLAG_CACHE_DISABLE = 0x10,
    MMU_PAGE_TABLE_FLAG_ACCESSED = 0x20,
    MMU_PAGE_TABLE_FLAG_DIRTY = 0x40,
    MMU_PAGE_TABLE_FLAG_PAT = 0x80,
    MMU_PAGE_TABLE_FLAG_GLOBAL = 0x100,
};

typedef uint32_t mmu_page_entry_t;

typedef struct {
    mmu_page_entry_t entries[PAGE_DIR_SIZE];
} __attribute__((packed)) mmu_page_dir_t;

typedef mmu_page_dir_t mmu_page_table_t;

mmu_page_dir_t * mmu_dir_create(void * addr);
void mmu_dir_free(mmu_page_dir_t * dir);

mmu_page_table_t * mmu_table_create(void * addr);
void mmu_table_free(mmu_page_table_t * table);

void mmu_dir_set_table(mmu_page_dir_t * dir, size_t i, mmu_page_table_t * table);
void mmu_dir_set_flags(mmu_page_dir_t * dir, size_t i, enum MMU_PAGE_DIR_FLAG flags);
/// This is in the virtual address space, use mmu_dir_get_paddr for physical address
mmu_page_table_t * mmu_dir_get_table(mmu_page_dir_t * dir, size_t i);
void mmu_dir_set(mmu_page_dir_t * dir, size_t i, mmu_page_table_t * table, enum MMU_PAGE_DIR_FLAG flags);

uint32_t mmu_dir_get_paddr(mmu_page_dir_t * dir, size_t i);
uint32_t mmu_dir_get_vaddr(mmu_page_dir_t * dir, size_t i);
enum MMU_PAGE_DIR_FLAG mmu_dir_get_flags(mmu_page_dir_t * dir, size_t i);

void mmu_table_set_addr(mmu_page_table_t * table, size_t i, uint32_t page_addr);
void mmu_table_set_flags(mmu_page_table_t * table, size_t i, enum MMU_PAGE_TABLE_FLAG flags);
void mmu_table_set(mmu_page_table_t * table, size_t i, uint32_t page_addr, enum MMU_PAGE_TABLE_FLAG flags);

uint32_t mmu_table_get_addr(mmu_page_table_t * table, size_t i);
enum MMU_PAGE_TABLE_FLAG mmu_table_get_flags(mmu_page_table_t * table, size_t i);

extern void mmu_enable_paging(mmu_page_dir_t * dir);
extern void mmu_disable_paging();
extern bool mmu_paging_enabled();
extern void mmu_change_dir(mmu_page_dir_t * dir);
extern mmu_page_dir_t * mmu_get_curr_dir();

// TODO way in the future, free interior pages when malloc free's entire virtual page

#endif // MMU_H
