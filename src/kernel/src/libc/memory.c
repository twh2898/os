#include "libc/memory.h"

#include "cpu/ram.h"
#include "kernel.h"
#include "libc/stdio.h"
#include "libc/string.h"

#define MEMORY_TABLE_ENTRY_COUNT ((PAGE_SIZE - 8) / sizeof(memory_table_entry_t)) // 511

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

mmu_page_dir_t * pdir;
size_t next_page;
memory_table_t * mtable;

static void * add_page();
static void init_table(memory_table_t * table, memory_table_t * prev, memory_table_t * next);

static void entry_set_addr(memory_table_entry_t * entry, uint32_t addr);
static void entry_set_flags(memory_table_entry_t * entry, enum MEMORY_ENTRY_FLAG flags);
static void entry_set_size(memory_table_entry_t * entry, size_t size);
static void entry_set(memory_table_entry_t * entry, uint32_t addr, size_t size, enum MEMORY_ENTRY_FLAG flags);
static void entry_copy(memory_table_entry_t * to, memory_table_entry_t * from);
static void entry_clear(memory_table_entry_t * entry);

static memory_table_entry_t * find_entry(memory_table_t * table, void * ptr);
static memory_table_entry_t * find_free(memory_table_t ** table, size_t * i, size_t size);

static inline memory_table_entry_t * entry_of(memory_table_t * table, size_t i);
static inline bool entry_is_free(memory_table_entry_t * entry);
static inline bool entry_is_present(memory_table_entry_t * entry);
static inline bool entry_is_present_free(memory_table_entry_t * entry);

static memory_table_entry_t * next_entry(memory_table_t * table, size_t i);

static size_t split_entry(memory_table_t * table, size_t i, size_t size);
static size_t merge_entry(memory_table_t * table, size_t i, size_t count);

void init_malloc(mmu_page_dir_t * dir, size_t first_page) {
    // kprintf("init_malloc(dir=%p page=%u)\n", dir, first_page);
    pdir = dir;
    next_page = first_page;

    mtable = add_page();
    // kprintf("First table is %p\n", mtable);
    init_table(mtable, 0, 0);
    // kprintf("init table done\n");
    void * first_free = add_page();
    // kprintf("First free page is %p\n", first_free);
    entry_set(entry_of(mtable, 0), PTR2UINT(first_free), PAGE_SIZE, MEMORY_ENTRY_FLAG_PRESENT | MEMORY_ENTRY_FLAG_FREE);
}

void * kmalloc(size_t size) {
    if (!size)
        return 0;

    size = PAGE_ALIGNED(size);

    memory_table_t * table = mtable;
    size_t i = 0;

    // Remember in find_free to merge adjacent free if they add up to size
    memory_table_entry_t * entry = find_free(&table, &i, size);

    if (!entry) {
        // table and i should be pointing to the one entry past the last present entry
        if (i >= MEMORY_TABLE_ENTRY_COUNT) {
            void * next_table = add_page();
            init_table(next_table, table, 0);
            table = next_table;
            i = 0;
        }

        void * next_free = add_page();
        size_t new_size = PAGE_SIZE;
        while (new_size < size) {
            add_page();
            size += PAGE_SIZE;
        }

        entry = entry_of(table, i);
        entry_set(entry, PTR2UINT(next_free), new_size, MEMORY_ENTRY_FLAG_PRESENT | MEMORY_ENTRY_FLAG_FREE);
    }

    if (entry->size >= size + PAGE_SIZE) {
        split_entry(table, i, size);
    }

    entry_set_flags(entry, MEMORY_ENTRY_FLAG_PRESENT);

    return UINT2PTR(entry->addr_flags & MASK_ADDR);
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
    // ram_page_alloc();
    // FIXME addr is 0x1000 if above is called first, need to figure out physical ram allocation bug
    void * ram_page = ram_page_alloc();
    // kprintf("add_page got page at physical address %p\n", ram_page);
    size_t table = next_page / 1024;
    size_t cell = next_page % 1024;

    void * page = UINT2PTR(next_page * PAGE_SIZE);

    // kprintf("table=%u cell=%u\n", table, cell);
    // kprintf("memory addr should be %p\n", page);

    // I'm pretty sure this is because mmu does not have a function to allocate
    // a second table. Need to check that.
    kassert_msg(table == 0, "MULTI TABLE NOT YET SUPPORTED");

    mmu_page_table_t * ptable = mmu_dir_get_table(pdir, table);
    mmu_table_set(ptable, cell, PTR2UINT(ram_page), MMU_TABLE_RW);

    // kprintf("try first write");
    // ((char *)page)[0] = 0;
    // kprintf(" done\n");

    next_page++;
    return page;
}

static void init_table(memory_table_t * table, memory_table_t * prev, memory_table_t * next) {
    // kprintf("init_table(table=%p prev=%p next=%p)\n", table, prev, next);
    table->prev = PTR2UINT(prev);
    // kprintf("set first ");
    table->next = PTR2UINT(next);
    // kprintf("set second ");
    kmemset(table->entries, 0, sizeof(table->entries));
    // kprintf("done\n");
}

static void entry_set_addr(memory_table_entry_t * entry, uint32_t addr) {
    // kprintf("entry_set_addr(entry=%p addr=%u)\n", entry, addr);
    if (!entry || !addr)
        return;

    uint32_t addr_flags = entry->addr_flags;
    uint32_t flags = addr_flags & MASK_FLAGS;
    entry->addr_flags = (addr & MASK_ADDR) | flags;
}

static void entry_set_flags(memory_table_entry_t * entry, enum MEMORY_ENTRY_FLAG flags) {
    // kprintf("entry_set_addr(entry=%p flags=%x)\n", entry, flags);
    if (!entry)
        return;

    uint32_t addr_flags = entry->addr_flags;
    uint32_t addr = addr_flags & MASK_ADDR;
    entry->addr_flags = addr | (flags & MASK_FLAGS);
}

static void entry_set_size(memory_table_entry_t * entry, size_t size) {
    // kprintf("entry_set_size(entry=%p size=%u)\n", entry, size);
    if (!entry)
        return;

    entry->size = size;
}

static void entry_set(memory_table_entry_t * entry, uint32_t addr, size_t size, enum MEMORY_ENTRY_FLAG flags) {
    // kprintf("entry_set_size(entry=%p addr=%u size=%u flags=%x)\n", entry, addr, size, flags);
    if (!entry || !addr)
        return;

    entry->addr_flags = (addr & MASK_ADDR) | (flags & MASK_FLAGS);
    entry->size = size;
}

static void entry_copy(memory_table_entry_t * to, memory_table_entry_t * from) {
    // kprintf("entry_copy(to=%p from=%p)\n", to, from);
    if (!to || !from)
        return;

    to->addr_flags = from->addr_flags;
    to->size = from->size;
}

static void entry_clear(memory_table_entry_t * entry) {
    // kprintf("entry_clear(entry=%p)\n", entry);
    entry->addr_flags = 0;
    entry->size = 0;
}

static memory_table_entry_t * find_entry(memory_table_t * table, void * ptr) {
    // kprintf("find_entry(table=%p ptr=%p)\n", table, ptr);
    if (!table || !ptr)
        return 0;

    while (table->prev) table = UINT2PTR(table->prev);

    size_t i = 0;
    memory_table_entry_t * entry = entry_of(table, i);

    while (entry) {
        if (entry->addr_flags & MASK_ADDR == PTR2UINT(ptr))
            return entry;

        i++;
        if (i >= MEMORY_TABLE_ENTRY_COUNT) {
            if (table->next) {
                table = UINT2PTR(table->next);
                i = 0;
            }
            else {
                break;
            }
        }

        entry = entry_of(table, i);
    }

    return 0;
}

static memory_table_entry_t * find_free(memory_table_t ** table, size_t * i, size_t size) {
    // kprintf("find_entry(table=%p *table=%p i=%u size=%u)\n", table, *table, *i, size);
    if (!table || !*table || !i)
        return 0;

    while ((*table)->prev) *table = UINT2PTR((*table)->prev);

    for (;;) {
        for (*i = 0; *i < MEMORY_TABLE_ENTRY_COUNT; (*i)++) {
            // memory_table_entry_t * entry = &(*table)->entries[*i];
            memory_table_entry_t * entry = entry_of(*table, *i);

            // Found end of entries, keep i at this spot
            if (!entry_is_present(entry)) {
                return 0;
            }

            // if (entry->addr_flags & (MEMORY_ENTRY_FLAG_PRESENT | MEMORY_ENTRY_FLAG_FREE)) {
            if (entry_is_free(entry)) {
                if (entry->size >= size)
                    return entry;

                memory_table_entry_t * next = next_entry(*table, *i);
                size_t total = entry->size;
                size_t count = 0;
                while (next && total < size && next->addr_flags & MEMORY_ENTRY_FLAG_FREE) {
                    count++;
                }
                merge_entry(*table, *i, count);
                return entry;
            }
        }

        // I should be set to entry count of table
        if (!(*table)->next)
            return 0;

        *table = UINT2PTR((*table)->next);
    }
}

static inline memory_table_entry_t * entry_of(memory_table_t * table, size_t i) {
    if (!table || i >= MEMORY_TABLE_ENTRY_COUNT)
        return 0;

    return &table->entries[i];
}

static inline bool entry_is_free(memory_table_entry_t * entry) {
    return entry->addr_flags & MEMORY_ENTRY_FLAG_FREE;
}

static inline bool entry_is_present(memory_table_entry_t * entry) {
    return entry->addr_flags & MEMORY_ENTRY_FLAG_PRESENT;
}

static inline bool entry_is_present_free(memory_table_entry_t * entry) {
    return entry->addr_flags & (MEMORY_ENTRY_FLAG_FREE | MEMORY_ENTRY_FLAG_PRESENT);
}

static memory_table_entry_t * next_entry(memory_table_t * table, size_t i) {
    // kprintf("next_entry(table=%p i=%u)\n", table, i);
    if (!table)
        return 0;

    if (i < MEMORY_TABLE_ENTRY_COUNT - 1) {
        // if (table->entries[i + 1].addr_flags & MEMORY_ENTRY_FLAG_PRESENT)
        memory_table_entry_t * entry = entry_of(table, i + 1);
        if (entry_is_present(entry))
            // return &table->entries[i + 1];
            return entry;
    }
    else if (table->next) {
        table = UINT2PTR(table->next);
        // if (table->entries[0].addr_flags & MEMORY_ENTRY_FLAG_PRESENT)
        memory_table_entry_t * entry = entry_of(table, 0);
        if (entry_is_present(entry))
            // return &table->entries[0];
            return entry;
    }

    return 0;
}

static size_t split_entry(memory_table_t * table, size_t i, size_t size) {
    // kprintf("split_entry(table=%p i=%u size=%u)\n", table, i, size);
    if (!table)
        return 0;

    kassert(i < MEMORY_TABLE_ENTRY_COUNT);

    memory_table_entry_t * entry = &table->entries[i];
    size = PAGE_ALIGNED(size);

    memory_table_t * currTable = table;
    size_t currI = i;

    while (next_entry(currTable, currI)) {
        if (currI >= MEMORY_TABLE_ENTRY_COUNT) {
            if (currTable->next) {
                currTable = UINT2PTR(currTable->next);
                currI = 0;
            }
        }
        else {
            currI++;
        }
    }

    memory_table_t * lastTable = currTable;
    size_t lastI = currI++;

    if (lastI >= MEMORY_TABLE_ENTRY_COUNT) {
        void * next_table = add_page();
        init_table(next_table, lastTable, 0);
        lastTable = next_table;
        lastI = 0;
    }

    // memory_table_entry_t * curr = &currTable->entries[currI];
    // memory_table_entry_t * last = &lastTable->entries[lastI];
    memory_table_entry_t * curr = entry_of(currTable, currI);
    memory_table_entry_t * last = entry_of(lastTable, lastI);

    // while lastEntry != entry
    while (curr != entry) {
        // last->addr_flags = curr->addr_flags;
        // last->size = curr->size;
        entry_copy(last, curr);

        lastTable = currTable;
        lastI = currI;

        if (currI == 0) {
            currTable = UINT2PTR(currTable->prev);
            currI = MEMORY_TABLE_ENTRY_COUNT;
        }

        currI--;
    }

    // WRONG --> entry_set(lastTable, lastI, entry->size - size, MEMORY_ENTRY_FLAG_PRESENT | MEMORY_ENTRY_FLAG_FREE);
    // entry->size = size;
    entry_set_size(entry_of(lastTable, lastI), entry->size - size);
    entry_set_flags(entry_of(lastTable, lastI), MEMORY_ENTRY_FLAG_PRESENT | MEMORY_ENTRY_FLAG_FREE);
    entry_set_size(entry, size);

    return size;
}

static size_t merge_entry(memory_table_t * table, size_t i, size_t count) {
    // kprintf("merge_entry(table=%p i=%u count=%u)\n", table, i, count);
    if (!table)
        return 0;

    // memory_table_entry_t * entry = &table->entries[i];
    memory_table_entry_t * entry = entry_of(table, i);

    if (count == 0)
        return 0;

    memory_table_t * nextTable = table;
    size_t nextI = i;

    size_t j;
    for (j = 0; j < count; j++) {
        memory_table_entry_t * next = next_entry(nextTable, nextI);
        // if (!next || !(next->addr_flags & MEMORY_ENTRY_FLAG_FREE)) {
        if (!next || !entry_is_free(next)) {
            break;
        }

        if (nextI >= MEMORY_TABLE_ENTRY_COUNT - 1) {
            nextTable = UINT2PTR(nextTable->next);
            nextI = 0;
        }
        else {
            nextI++;
        }
    }

    count = j;

    // memory_table_entry_t * next = &nextTable->entries[nextI];
    memory_table_entry_t * next = entry_of(nextTable, nextI);

    while (entry) {
        if (next) {
            // entry->addr_flags = next->addr_flags;
            // entry->size = next->size;
            entry_copy(entry, next);

            nextI++;
            if (nextI >= MEMORY_TABLE_ENTRY_COUNT) {
                if (nextTable->next) {
                    nextTable = UINT2PTR(nextTable->next);
                    nextI = 0;

                    next = &nextTable->entries[nextI];
                }
                else {
                    next = NULL;
                }
            }

            // if (next && !(next->addr_flags & MEMORY_ENTRY_FLAG_PRESENT)) {
            if (next && !entry_is_present(next)) {
                next = NULL;
            }
        }
        else {
            // entry->addr_flags = 0;
            // entry->size = 0;
            entry_clear(entry);
        }

        i++;
        if (i > MEMORY_TABLE_ENTRY_COUNT) {
            if (table->next) {
                table = UINT2PTR(table->next);
                i = 0;

                entry = &table->entries[i];
            }
            else {
                entry = NULL;
            }
        }

        // if (entry && !(entry->addr_flags & MEMORY_ENTRY_FLAG_PRESENT)) {
        if (entry && !entry_is_present(entry)) {
            entry = NULL;
        }
    }

    return count;
}
