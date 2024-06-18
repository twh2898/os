#include "libc/memory.h"

#include "drivers/ram.h"
#include "kernel.h"
#include "libc/string.h"

#define MEMORY_TABLE_ENTRY_COUNT \
    ((PAGE_SIZE - 8) / sizeof(memory_table_entry_t)) // 511

enum MEMORY_ENTRY_FLAG {
    MEMORY_ENTRY_FLAG_PRESENT = 0x1,
    MEMORY_ENTRY_FLAG_FREE = 0x2,
};

typedef struct {
    uint32_t addr_flags;
    uint32_t size;
} memory_table_entry_t;

typedef struct {
    uint32_t next;
    uint32_t prev;
    memory_table_entry_t entries[MEMORY_TABLE_ENTRY_COUNT];
} memory_table_t;

mmu_page_dir_t * pdir = 0;
size_t next_page = 0;
memory_table_t * mtable;

static void * add_page();
static void init_table(memory_table_t * table, uint32_t prev, uint32_t next);
static memory_table_entry_t * find_entry(memory_table_t * table, void * ptr);

void init_malloc(mmu_page_dir_t * dir, size_t first_page) {
    pdir = dir;
    next_page = first_page;

    mtable = add_page();
    init_table(mtable, 0, 0);
}

void * kmalloc(size_t size) {
    if (!size)
        return 0;

    size = PAGE_ALIGNED(size);

    // TODO find next free memory of at least size
    memory_table_entry_t * entry = 0;
    if (!entry) {
        // TODO request more pages
        // TODO check if no more physical memory is available
        return 0;
    }

    if (entry->size < size) {
        // TODO find next free and loop
    }

    if (entry->size >= size * 2) {
        // TODO if next is free, move extra space from current to next
        // TODO if entry is last, create a new entry after for remaining space
    }

    // TODO if end found, request more physical pages, add to virtual memory
    // TODO check if no more physical memory is available

    entry->addr_flags &= ~MEMORY_ENTRY_FLAG_FREE;

    return entry->addr_flags & MASK_FLAGS;
}

void kfree(void * ptr) {
    if (!ptr)
        return;

    memory_table_entry_t * entry = find_entry(mtable, ptr);
    if (entry) {
        entry->addr_flags |= MEMORY_ENTRY_FLAG_FREE;
    }
}

static void * add_page() {
    void * page = ram_page_alloc();
    size_t table = next_page / 1024;
    size_t cell = next_page % 1024;

    if (table >= 1)
        KERNEL_PANIC("MULTI TABLE NOT YET SUPPORTED");

    mmu_page_table_t * ptable = mmu_dir_get_table(pdir, table);
    mmu_table_set_addr(ptable, cell, (uint32_t)page);
    mmu_table_set_flags(ptable, cell, MMU_TABLE_RW);

    next_page++;
    return page;
}

static void init_table(memory_table_t * table, uint32_t prev, uint32_t next) {
    table->prev = prev;
    table->next = next;
    memset(table->entries, 0, sizeof(table->entries));
}

static memory_table_entry_t * find_entry(memory_table_t * table, void * ptr) {
    if (!table || !ptr)
        return 0;

    while (table->prev) table = (memory_table_t *)table->prev;

    while (table) {
        for (size_t i = 0; i < MEMORY_TABLE_ENTRY_COUNT; i++) {
            memory_table_entry_t * entry = &table->entries[i];

            // First non-present page is end of all tables
            if (!(entry->addr_flags & MEMORY_ENTRY_FLAG_PRESENT))
                return 0;

            if (entry->addr_flags & MASK_ADDR == PTR2UINT(ptr))
                return entry;
        }
        table = (memory_table_t *)table->next;
    }

    return 0;
}
