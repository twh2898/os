#include "cpu/ram.h"

#include "cpu/boot_params.h"
#include "defs.h"
#include "libc/process.h"
#include "libc/stdio.h"
#include "libc/string.h"

#define REGION_TABLE_SIZE     (PAGE_SIZE / sizeof(region_table_entry_t)) // 512
#define REGION_MAX_PAGE_COUNT 0x8000
#define REGION_MAX_SIZE       (REGION_MAX_PAGE_COUNT * PAGE_SIZE)

#define REGION_TABLE_FLAG_PRESENT 0x1
#define BITMASK_PAGE_FREE         0x1

typedef struct {
    uint32_t addr_flags;
    uint16_t page_count;
    uint16_t free_count;
} __attribute__((packed)) region_table_entry_t;

typedef struct {
    region_table_entry_t entries[REGION_TABLE_SIZE];
} __attribute__((packed)) region_table_t;

region_table_t * region_table;

// THIS IS IN PHYSICAL ADDRESS SPACE NOT VIRTUAL
static size_t build_table();
static void   create_bitmask(size_t region_index);
static void   set_bitmask_early(size_t region_index, uint16_t bit, bool free);
// END THIS IS IN PHYSICAL ADDRESS SPACE NOT VIRTUAL

static void set_bitmask(size_t region_index, uint16_t bit, bool free);
static bool get_bitmask(size_t region_index, uint16_t bit);
// Returns 0 for error, 0 is always invalid
static uint16_t find_free_bit(size_t region_index);
static size_t   find_addr_entry(uint32_t addr);
// Returns 0 for error, 0 is always invalid
static size_t find_addr_bit(size_t region_index, uint32_t addr);

static uint16_t      lower_ram;
static uint16_t      upper_ram_count;
static upper_ram_t * upper_ram;

static void sort_ram();

void ram_init(void * ram_table, size_t * ram_table_count) {
    region_table = (region_table_t *)ram_table;
    kmemset(region_table, 0, sizeof(region_table_t));

    boot_params_t * bparams = get_boot_params();

    lower_ram       = bparams->low_mem_size;
    upper_ram_count = bparams->mem_entries_count;
    upper_ram       = bparams->mem_entries;
    sort_ram();

    *ram_table_count = build_table();
}

uint16_t ram_lower_size() {
    return lower_ram;
}

uint16_t ram_upper_count() {
    return upper_ram_count;
}

uint64_t ram_upper_start(uint16_t i) {
    return upper_ram[i].base_addr;
}

uint64_t ram_upper_end(uint16_t i) {
    return upper_ram[i].base_addr + upper_ram[i].length;
}

uint64_t ram_upper_size(uint16_t i) {
    return upper_ram[i].length;
}

bool ram_upper_usable(uint16_t i) {
    return upper_ram[i].type == RAM_TYPE_USABLE || upper_ram[i].type == RAM_TYPE_ACPI_RECLAIMABLE;
}

enum RAM_TYPE ram_upper_type(uint16_t i) {
    boot_params_t * bparams = get_boot_params();
    return bparams->mem_entries[i].type;
}

uint32_t ram_bitmask_paddr(size_t region_index) {
    if (region_index <= REGION_TABLE_SIZE)
        return region_table->entries[region_index].addr_flags & MASK_ADDR;
    return 0;
}

uint32_t ram_bitmask_vaddr(size_t region_index) {
    if (region_index <= REGION_TABLE_SIZE)
        return VADDR_RAM_BITMASKS + region_index * PAGE_SIZE;
    return 0;
}

uint32_t ram_page_alloc() {
    for (size_t i = 0; i < REGION_TABLE_SIZE; i++) {
        uint8_t flag = region_table->entries[i].addr_flags & MASK_FLAGS;
        if (flag & REGION_TABLE_FLAG_PRESENT && region_table->entries[i].free_count) {
            uint16_t bit = find_free_bit(i);
            if (!bit)
                PANIC("Could not find free bit in page with free count");

            set_bitmask(i, bit, false);
            region_table->entries[i].free_count--;
            uint32_t page_addr = ram_bitmask_paddr(i) + bit * PAGE_SIZE;
            return page_addr;
        }
    }

    return 0;
}

uint32_t ram_page_palloc() {
    for (size_t i = 0; i < REGION_TABLE_SIZE; i++) {
        uint8_t flag = region_table->entries[i].addr_flags & MASK_FLAGS;
        if (flag & REGION_TABLE_FLAG_PRESENT && region_table->entries[i].free_count) {
            uint16_t bit = find_free_bit(i);
            if (!bit)
                PANIC("Could not find free bit in page with free count");

            set_bitmask_early(i, bit, false);
            region_table->entries[i].free_count--;
            uint32_t page_addr = ram_bitmask_paddr(i) + bit * PAGE_SIZE;
            return page_addr;
        }
    }

    return 0;
}

void ram_page_free(uint32_t addr) {
    size_t                 region_index = find_addr_entry(addr);
    region_table_entry_t * region       = &region_table->entries[region_index];
    if (!region)
        return;

    uint16_t bit = find_addr_bit(region_index, addr);
    if (!bit)
        return;

    set_bitmask(region_index, bit, true);
    region->free_count++;
}

static void sort_ram() {
    upper_ram_t swap;

    // TODO handle overlap
    for (size_t i = 0; i < upper_ram_count - 1; i++) {
        uint64_t curr_start = upper_ram[i].base_addr;

        size_t   next_i     = i;
        uint64_t next_start = curr_start;

        for (size_t i = i; i < upper_ram_count; i++) {
            if (upper_ram[i].base_addr > next_start) {
                next_i     = i;
                next_start = upper_ram[i].base_addr;
            }
        }

        if (next_i != i) {
            swap              = upper_ram[i];
            upper_ram[i]      = upper_ram[next_i];
            upper_ram[next_i] = upper_ram[i];
        }
    }
}

// THIS IS IN PHYSICAL ADDRESS SPACE NOT VIRTUAL
static size_t build_table() {
    size_t first_area = 0;
    bool   found_free = false;

    for (size_t i = 0; i < ram_upper_count(); i++) {
        uint32_t addr = (uint32_t)ram_upper_start(i);
        if (ram_upper_usable(i) && addr > 0x9fbff) {
            if (addr & MASK_FLAGS)
                addr += PAGE_SIZE;
            found_free = true;
            first_area = i;
            break;
        }
    }

    if (!found_free) {
        PANIC("FAILED TO FIND FREE RAM");
    }

    uint32_t first_free_size = ram_upper_end(first_area) - ram_upper_start(first_area);

    // First 2 pages (first page table, last page table)
    if (first_free_size < PAGE_SIZE * 4) {
        PANIC("FAILED TO FIND FREE RAM");
    }

    size_t table_index = 0;
    for (size_t i = first_area; i < ram_upper_count(); i++) {
        if (!ram_upper_usable(i))
            continue;

        uint32_t start = (uint32_t)ram_upper_start(i);
        uint32_t end   = (uint32_t)ram_upper_end(i) & MASK_ADDR;
        uint32_t len   = end - start;

        start &= MASK_ADDR;

        size_t region_count = len / REGION_MAX_SIZE;
        if (len % REGION_MAX_SIZE)
            region_count++;

        for (size_t r = 0; r < region_count; r++) {
            if (table_index >= REGION_TABLE_SIZE)
                PANIC("Region table overflow");

            size_t region_len = len;
            if (region_len > REGION_MAX_SIZE)
                region_len = REGION_MAX_SIZE;
            start &= MASK_ADDR;
            size_t region_end = (start + region_len);

            region_table_entry_t * region = &region_table->entries[table_index];

            region->addr_flags = start | REGION_TABLE_FLAG_PRESENT;
            region->page_count = region_len / PAGE_SIZE;
            region->free_count = region->page_count - 1; // -1 for bitmask page

            create_bitmask(table_index);

            len -= region_len;
            start += region_len;

            table_index++;
        }
    }

    return table_index;
}

static void create_bitmask(size_t region_index) {
    if (region_index > REGION_TABLE_SIZE)
        return;

    region_table_entry_t * region = &region_table->entries[region_index];

    uint8_t * bitmask = (uint8_t *)(region->addr_flags & MASK_ADDR);

    size_t pages = region->page_count;
    size_t bytes = pages / 8;
    size_t bits  = pages % 8;

    kmemset(bitmask, 0, PAGE_SIZE);
    kmemset(bitmask, 0xff, bytes);

    for (size_t i = 0; i < bits; i++) {
        bitmask[bytes] |= 1 << i;
    }

    // Set bitmask page as used
    bitmask[0] &= 0xfe;
}

static void set_bitmask_early(size_t region_index, uint16_t bit, bool free) {
    if (region_index > REGION_TABLE_SIZE)
        return;

    size_t byte = bit / 8;
    bit         = bit % 8;

    uint8_t * bitmask = UINT2PTR(ram_bitmask_paddr(region_index));

    if (free) {
        bitmask[byte] |= 1 << bit;
    }
    else {
        bitmask[byte] &= ~(1 << bit);
    }
}
// END THIS IS IN PHYSICAL ADDRESS SPACE NOT VIRTUAL

static void set_bitmask(size_t region_index, uint16_t bit, bool free) {
    if (region_index > REGION_TABLE_SIZE)
        return;

    uint8_t * bitmask = UINT2PTR(ram_bitmask_vaddr(region_index));
    size_t    byte    = bit / 8;
    bit               = bit % 8;

    if (free) {
        bitmask[byte] |= 1 << bit;
    }
    else {
        bitmask[byte] &= ~(1 << bit);
    }
}

static bool get_bitmask(size_t region_index, uint16_t bit) {
    if (region_index > REGION_TABLE_SIZE)
        return false;

    uint8_t * bitmask = UINT2PTR(ram_bitmask_vaddr(region_index));
    size_t    byte    = bit / 8;
    bit               = bit % 8;

    // Clamp true to 1
    return (bitmask[byte] & (1 << bit)) >> bit;
}

// Returns 0 on failure, 0 is always invalid
static uint16_t find_free_bit(size_t region_index) {
    if (region_index > REGION_TABLE_SIZE)
        return 0;

    region_table_entry_t * region = &region_table->entries[region_index];
    if (region->free_count == 0)
        return 0;

    for (uint16_t i = 1; i < region->page_count; i++) {
        if (get_bitmask(region_index, i))
            return i;
    }

    return 0;
}

static size_t find_addr_entry(uint32_t addr) {
    if (addr & MASK_FLAGS)
        return 0;

    for (size_t i = 0; i < REGION_TABLE_SIZE; i++) {
        if (region_table->entries[i].addr_flags & REGION_TABLE_FLAG_PRESENT) {
            uint32_t region_start = ram_bitmask_paddr(i);
            uint32_t region_end   = region_start + region_table->entries[i].page_count * PAGE_SIZE;

            if (addr >= region_start && addr <= region_end)
                &region_table->entries[i];
        }
    }

    return 0;
}

// Returns 0 for error, 0 is always invalid
static size_t find_addr_bit(size_t region_index, uint32_t addr) {
    if (region_index > REGION_TABLE_SIZE)
        return 0;

    region_table_entry_t * region = &region_table->entries[region_index];
    if (addr & MASK_FLAGS || !(region->addr_flags & REGION_TABLE_FLAG_PRESENT))
        return 0;

    uint32_t region_start = region->addr_flags & MASK_ADDR;
    uint32_t region_end   = region_start + region->page_count * PAGE_SIZE;

    if (addr <= region_start || addr >= region_end)
        return 0;

    return (addr - region_start) / PAGE_SIZE;
}
