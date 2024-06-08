#ifndef MMU_H
#define MMU_H

#include <stdbool.h>
#include <stdint.h>

#define PAGE_DIR_SIZE 1024
#define PAGE_TABLE_SIZE 1024

enum MMU_PAGE_DIR_FLAGS {
    MMU_PAGE_DIR_FLAGS_PRESENT = 0x1,
    MMU_PAGE_DIR_FLAGS_READ_WRITE = 0x2,
    MMU_PAGE_DIR_FLAGS_USER_SUPERVISOR = 0x4,
    MMU_PAGE_DIR_FLAGS_WRITE_THROUGH = 0x8,
    MMU_PAGE_DIR_FLAGS_CACHE_DISABLE = 0x10,
    MMU_PAGE_DIR_FLAGS_ACCESSED = 0x20,
    MMU_PAGE_DIR_FLAGS_PAGE_SIZE = 0x80,
};

enum MMU_PAGE_TABLE_FLAGS {
    MMU_PAGE_TABLE_FLAGS_PRESENT = 0x1,
    MMU_PAGE_TABLE_FLAGS_READ_WRITE = 0x2,
    MMU_PAGE_TABLE_FLAGS_USER_SUPERVISOR = 0x4,
    MMU_PAGE_TABLE_FLAGS_WRITE_THROUGH = 0x8,
    MMU_PAGE_TABLE_FLAGS_CACHE_DISABLE = 0x10,
    MMU_PAGE_TABLE_FLAGS_ACCESSED = 0x20,
    MMU_PAGE_TABLE_FLAGS_DIRTY = 0x40,
    MMU_PAGE_TABLE_FLAGS_PAT = 0x80,
    MMU_PAGE_TABLE_FLAGS_GLOBAL = 0x100,
};

typedef struct {
    uint16_t flags;
    uint16_t addr;
} __attribute__((packed)) mmu_page_entry_t;

typedef struct {
    mmu_page_entry_t entries[PAGE_DIR_SIZE];
} __attribute__((packed)) mmu_page_dir_t;

typedef mmu_page_dir_t mmu_page_table_t;

extern void mmu_enable_paging(mmu_page_dir_t * dir);
extern void mmu_disable_paging();
extern bool mmu_paging_enabled();
extern void mmu_change_dir(mmu_page_dir_t * dir);
extern mmu_page_dir_t * mmu_get_curr_dir();

// TODO re-enable when you actually have page dir mapped
// void * mmu_phys_addr(void * virt_addr);
// TODO re-enable when you actually have paging working
// void mmu_map_page(mmu_page_dir_t * dir, void *phys_addr, void *virtual_addr, unsigned int flags);

// TODO request page increase
// TODO request page decrease

// TODO way in the future, free interior pages when malloc free's entire virtual
// page,

#endif // MMU_H
