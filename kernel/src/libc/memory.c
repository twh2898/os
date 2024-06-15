#include "libc/memory.h"

#include "drivers/page.h"
#include "kernel.h"

#define MEMORY_TABLE_ENTRY_COUNT 1022

typedef uint32_t memory_table_entry_t;

typedef struct {
    uint32_t next;
    uint32_t prev;
    memory_table_entry_t entries[MEMORY_TABLE_ENTRY_COUNT];
} __attribute__((packed)) memory_table_t;

mmu_page_dir_t * pdir = 0;
size_t next_page = 0;
memory_table_t * mtable;

static void * add_page();
static mmu_page_table_t * add_table();

void init_malloc(mmu_page_dir_t * dir, size_t first_page) {
    pdir = dir;
    next_page = first_page;

    // TODO init first malloc table
    mtable = add_page();
}

void * kmalloc(size_t size) {
    return 0;
}

void kfree(void * ptr) {}

static void * add_page() {
    void * page = page_alloc();
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
