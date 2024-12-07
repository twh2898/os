#include "cpu/mmu.h"

#include "cpu/ram.h"
#include "kernel.h"
#include "libc/string.h"

mmu_page_dir_t * mmu_dir_create(void * addr) {
    mmu_page_dir_t * dir = addr;
    if (dir) {
        kmemset(dir, 0, sizeof(mmu_page_dir_t));
    }
    return dir;
}

void mmu_dir_free(mmu_page_dir_t * dir) {
    if (!dir)
        return;

    if (dir) {
        for (size_t i = 0; i < PAGE_DIR_SIZE; i++) {
            if (dir->entries[i] & MMU_PAGE_DIR_FLAG_PRESENT) {
                uint32_t table_addr = dir->entries[i] & MASK_ADDR;
                mmu_table_free((mmu_page_table_t *)table_addr);
            }
        }
    }
    ram_page_free(dir);
}

mmu_page_table_t * mmu_table_create(void * addr) {
    mmu_page_table_t * table = addr;
    if (table) {
        kmemset(table, 0, sizeof(mmu_page_table_t));
    }
    return table;
}

void mmu_table_free(mmu_page_table_t * table) {
    if (!table)
        return;

    if (table) {
        for (size_t i = 0; i < PAGE_TABLE_SIZE; i++) {
            if (table->entries[i] & MMU_PAGE_TABLE_FLAG_PRESENT) {
                uint32_t page_addr = table->entries[i] & MASK_ADDR;
                ram_page_free((void *)page_addr);
            }
        }
    }
    ram_page_free(table);
}

void mmu_dir_set_table(mmu_page_dir_t * dir, size_t i, mmu_page_table_t * table) {
    if (!dir || !table || i >= PAGE_DIR_SIZE)
        return;

    mmu_page_entry_t entry = dir->entries[i];
    entry &= MASK_FLAGS;
    entry |= ((uint32_t)table) & MASK_ADDR;

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

void mmu_dir_set(mmu_page_dir_t * dir, size_t i, mmu_page_table_t * table, enum MMU_PAGE_DIR_FLAG flags) {
    if (!dir || i >= PAGE_DIR_SIZE)
        return;

    mmu_page_entry_t entry = (PTR2UINT(table) & MASK_ADDR) | (flags & MASK_FLAGS);
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
