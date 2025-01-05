#include "cpu/mmu.h"

#include "libc/process.h"
#include "libc/string.h"

void mmu_dir_clear(mmu_dir_t * dir) {
    kmemset(dir, 0, sizeof(mmu_dir_t));
}

void mmu_table_clear(mmu_table_t * table) {
    kmemset(table, 0, sizeof(mmu_table_t));
}

int mmu_dir_set_addr(mmu_dir_t * dir, size_t i, uint32_t addr) {
    if (!dir || i >= MMU_DIR_SIZE) {
        return -1;
    }

    mmu_entry_t entry = dir->entries[i];
    entry &= MASK_FLAGS;
    entry |= addr & MASK_ADDR;

    dir->entries[i] = entry;

    return 0;
}

int mmu_dir_set_flags(mmu_dir_t * dir, size_t i, enum MMU_DIR_FLAG flags) {
    if (!dir || i >= MMU_DIR_SIZE) {
        return -1;
    }

    mmu_entry_t entry = dir->entries[i];
    entry &= MASK_ADDR;
    entry |= flags & MASK_FLAGS;

    dir->entries[i] = entry;

    return 0;
}

void mmu_dir_set(mmu_dir_t * dir, size_t i, uint32_t addr, enum MMU_DIR_FLAG flags) {
    if (!dir || i >= MMU_DIR_SIZE) {
        return;
    }

    mmu_entry_t entry = (addr & MASK_ADDR) | (flags & MASK_FLAGS);

    dir->entries[i] = entry;
}

uint32_t mmu_dir_get_addr(mmu_dir_t * dir, size_t i) {
    if (!dir || i >= MMU_DIR_SIZE) {
        return 0;
    }

    return dir->entries[i] & MASK_ADDR;
}

enum MMU_DIR_FLAG mmu_dir_get_flags(mmu_dir_t * dir, size_t i) {
    if (!dir || i >= MMU_DIR_SIZE) {
        return 0;
    }

    return dir->entries[i] & MASK_FLAGS;
}

void mmu_table_set_addr(mmu_table_t * table, size_t i, uint32_t addr) {
    if (!table || i >= MMU_TABLE_SIZE) {
        return;
    }

    mmu_entry_t entry = table->entries[i];
    entry &= MASK_FLAGS;
    entry |= addr & MASK_ADDR;

    table->entries[i] = entry;
}

void mmu_table_set_flags(mmu_table_t * table, size_t i, enum MMU_TABLE_FLAG flags) {
    if (!table || i >= MMU_TABLE_SIZE) {
        return;
    }

    mmu_entry_t entry = table->entries[i];
    entry &= MASK_ADDR;
    entry |= flags & MASK_FLAGS;

    table->entries[i] = entry;
}

void mmu_table_set(mmu_table_t * table, size_t i, uint32_t addr, enum MMU_TABLE_FLAG flags) {
    if (!table || i >= MMU_TABLE_SIZE) {
        return;
    }

    mmu_entry_t entry = (addr & MASK_ADDR) | (flags & MASK_FLAGS);

    table->entries[i] = entry;
}

uint32_t mmu_table_get_addr(mmu_table_t * table, size_t i) {
    if (!table || i >= MMU_TABLE_SIZE) {
        return 0;
    }

    return table->entries[i] & MASK_ADDR;
}

enum MMU_TABLE_FLAG mmu_table_get_flags(mmu_table_t * table, size_t i) {
    if (!table || i >= MMU_TABLE_SIZE) {
        return 0;
    }

    return table->entries[i] & MASK_FLAGS;
}
