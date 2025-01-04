#include "cpu/mmu.h"

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
            mmu_table_clear((mmu_page_table_t *)table_addr);
        }
    }

    kmemset(dir, 0, sizeof(mmu_page_dir_t));
}

mmu_page_table_t * mmu_table_create(void * addr) {
    mmu_page_table_t * table = addr;
    if (table) {
        mmu_table_clear(table);
    }
    return table;
}

void mmu_table_clear(mmu_page_table_t * table) {
    if (!table)
        return;

    kmemset(table, 0, sizeof(mmu_page_table_t));
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
