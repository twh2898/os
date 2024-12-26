#include "cpu/mmu.h"

#include "cpu/ram.h"
#include "libc/process.h"
#include "libc/string.h"

mmu_page_dir_t * mmu_dir_create(void * addr) {
    mmu_page_dir_t * dir = addr;
    if (dir) {
        kmemset(dir, 0, sizeof(mmu_page_dir_t));
    }
    return dir;
}

void mmu_dir_clear(mmu_page_dir_t * dir) {
    if (!dir)
        return;

    for (size_t i = 0; i < PAGE_DIR_SIZE; i++) {
        if (dir->entries[i] & MMU_PAGE_DIR_FLAG_PRESENT) {
            uint32_t table_addr = dir->entries[i] & MASK_ADDR;
            mmu_table_free((mmu_page_table_t *)table_addr);
        }
    }
}

void mmu_dir_free(mmu_page_dir_t * dir) {
    mmu_dir_clear(dir);
    ram_page_free(PTR2UINT(dir));
}

mmu_page_table_t * mmu_table_create(void * addr) {
    mmu_page_table_t * table = addr;
    if (table) {
        kmemset(table, 0, sizeof(mmu_page_table_t));
    }
    return table;
}

void mmu_table_clear(mmu_page_table_t * table) {
    if (!table)
        return;

    for (size_t i = 0; i < PAGE_TABLE_SIZE; i++) {
        if (table->entries[i] & MMU_PAGE_TABLE_FLAG_PRESENT) {
            uint32_t page_addr = table->entries[i] & MASK_ADDR;
            ram_page_free(page_addr);
        }
    }
}

void mmu_table_free(mmu_page_table_t * table) {
    mmu_table_clear(table);
    ram_page_free(PTR2UINT(table));
}

void mmu_dir_set_table(mmu_page_dir_t * dir, size_t i, uint32_t table_addr) {
    if (!dir || i >= PAGE_DIR_SIZE)
        return;

    mmu_page_entry_t entry = dir->entries[i];
    entry &= MASK_FLAGS;
    entry |= table_addr & MASK_ADDR;

    dir->entries[i] = entry;
}

void mmu_dir_set_flags(mmu_page_dir_t * dir, size_t i, enum MMU_PAGE_DIR_FLAG flags) {
    if (!dir || i >= PAGE_DIR_SIZE)
        return;

    mmu_page_entry_t entry = dir->entries[i];
    entry &= MASK_ADDR;
    entry |= flags & MASK_FLAGS;

    dir->entries[i] = entry;
}

mmu_page_table_t * mmu_dir_get_table(mmu_page_dir_t * dir, size_t i) {
    if (!dir || i >= PAGE_DIR_SIZE)
        return 0;

    uint32_t virtual_address = mmu_dir_get_vaddr(dir, i);

    return UINT2PTR(virtual_address);
}

void mmu_dir_set(mmu_page_dir_t * dir, size_t i, uint32_t table_addr, enum MMU_PAGE_DIR_FLAG flags) {
    if (!dir || i >= PAGE_DIR_SIZE)
        return;

    mmu_page_entry_t entry = (table_addr & MASK_ADDR) | (flags & MASK_FLAGS);

    dir->entries[i] = entry;
}

uint32_t mmu_dir_get_paddr(mmu_page_dir_t * dir, size_t i) {
    if (!dir || i >= PAGE_DIR_SIZE)
        return 0;

    return dir->entries[i] & MASK_ADDR;
}

uint32_t mmu_dir_get_vaddr(mmu_page_dir_t * dir, size_t i) {
    if (!dir || i >= PAGE_DIR_SIZE)
        return 0;

    return VADDR_FIRST_PAGE_TABLE + (i << 12);
}

enum MMU_PAGE_DIR_FLAG mmu_dir_get_flags(mmu_page_dir_t * dir, size_t i) {
    if (!dir || i >= PAGE_DIR_SIZE)
        return 0;

    return dir->entries[i] & MASK_FLAGS;
}

void mmu_dir_map_paddr(mmu_page_dir_t *         dir,
                       uint32_t                 vaddr,
                       uint32_t                 paddr,
                       enum MMU_PAGE_DIR_FLAG   dir_flags,
                       enum MMU_PAGE_TABLE_FLAG table_flags) {
    size_t table_i = ADDR2PAGE(vaddr);
    size_t dir_i   = table_i / 1024;
    table_i        = table_i % 1024;

    if (!(mmu_dir_get_flags(dir, table_i) & MMU_PAGE_DIR_FLAG_PRESENT)) {
        uint32_t new_table = ram_page_alloc();
        mmu_dir_set(dir, dir_i, new_table, dir_flags);
    }
    mmu_dir_set_flags(dir, dir_i, dir_flags);

    mmu_page_table_t * table = mmu_dir_get_table(dir, dir_i);
    mmu_table_set(table, table_i, paddr, table_flags);
}

void mmu_table_set_addr(mmu_page_table_t * table, size_t i, uint32_t page_addr) {
    if (!table || i >= PAGE_TABLE_SIZE)
        return;

    mmu_page_entry_t entry = table->entries[i];
    entry &= MASK_FLAGS;
    entry |= page_addr & MASK_ADDR;

    table->entries[i] = entry;
}

void mmu_table_set_flags(mmu_page_table_t * table, size_t i, enum MMU_PAGE_TABLE_FLAG flags) {
    if (!table || i >= PAGE_TABLE_SIZE)
        return;

    mmu_page_entry_t entry = table->entries[i];
    entry &= MASK_ADDR;
    entry |= flags & MASK_FLAGS;

    table->entries[i] = entry;
}

void mmu_table_set(mmu_page_table_t * table, size_t i, uint32_t page_addr, enum MMU_PAGE_TABLE_FLAG flags) {
    if (!table || i >= PAGE_TABLE_SIZE)
        return;

    mmu_page_entry_t entry = (page_addr & MASK_ADDR) | (flags & MASK_FLAGS);

    table->entries[i] = entry;
}

uint32_t mmu_table_get_addr(mmu_page_table_t * table, size_t i) {
    if (!table || i >= PAGE_TABLE_SIZE)
        return 0;

    return table->entries[i] & MASK_ADDR;
}

enum MMU_PAGE_TABLE_FLAG mmu_table_get_flags(mmu_page_table_t * table, size_t i) {
    if (!table || i >= PAGE_TABLE_SIZE)
        return 0;

    return table->entries[i] & MASK_FLAGS;
}
