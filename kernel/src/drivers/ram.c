#include "drivers/ram.h"

#include "kernel.h"
#include "libc/stdio.h"
#include "libc/string.h"

#define REGION_TABLE_ADDR 0x2000
#define REGION_TABLE_SIZE (PAGE_SIZE / sizeof(region_table_entry_t)) // 512
#define REGION_MAX_PAGE_COUNT 0x8000
#define REGION_MAX_SIZE (REGION_MAX_PAGE_COUNT * PAGE_SIZE)
#define REGION_VIRT_BITMASK_ADDR 0x9f000

#define REGION_TABLE_FLAG_PRESENT 0x1
#define BITMASK_PAGE_FREE 0x1

typedef struct {
    uint32_t addr_flags;
    uint16_t page_count;
    uint16_t free_count;
} __attribute__((packed)) region_table_entry_t;

typedef struct {
    region_table_entry_t entries[REGION_TABLE_SIZE];
} __attribute__((packed)) region_table_t;

region_table_t * region_table;

static void create_bitmask(size_t region_index);
static void set_physical_bitmask(size_t region_index, uint16_t bit, bool free);
static void set_bitmask(size_t region_index, uint16_t bit, bool free);
static bool get_bitmask(size_t region_index, uint16_t bit);
// Returns 0 for error, 0 is always invalid
static uint16_t find_free_bit(size_t region_index);
static size_t find_addr_entry(uint32_t addr);
// Returns 0 for error, 0 is always invalid
static size_t find_addr_bit(size_t region_index, uint32_t addr);

#define LOWER_RAM_ADDR 0x0500
#define UPPER_RAM_COUNT 0x0502
#define UPPER_RAM_ADDR 0x0504

typedef struct {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t ext;
} __attribute__((packed)) upper_ram_t;

static uint16_t lower_ram;
static uint16_t upper_ram_count;
static upper_ram_t * upper_ram;

static void sort_ram();

void init_ram() {
    lower_ram = *(uint16_t *)LOWER_RAM_ADDR;
    upper_ram_count = *(uint16_t *)UPPER_RAM_COUNT;
    upper_ram = (upper_ram_t *)UPPER_RAM_ADDR;
    sort_ram();

    size_t first_area = 0;
    bool found_free = false;

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
        KERNEL_PANIC("FAILED TO FIND FREE RAM");
    }

    // kprintf("Found available at index %u of %u\n", first_area,
    // ram_upper_count()); kprintf("Region table at %p\n", region_table);

    region_table = (region_table_t *)REGION_TABLE_ADDR;
    memset(region_table, 0, sizeof(region_table_t));

    size_t table_index = 0;
    for (size_t i = first_area; i < ram_upper_count(); i++) {
        if (!ram_upper_usable(i))
            continue;

        uint32_t start = (uint32_t)ram_upper_start(i);
        uint32_t end = (uint32_t)ram_upper_end(i) & MASK_ADDR;
        uint32_t len = end - start;

        if (!table_index)
            start += PAGE_SIZE;
        start &= MASK_ADDR;

        // kprintf("Ram region %u is usable %p - %p\n", i, start, end);

        size_t region_count = len / REGION_MAX_SIZE;
        if (len % REGION_MAX_SIZE)
            region_count++;

        for (size_t r = 0; r < region_count; r++) {
            if (table_index >= REGION_TABLE_SIZE)
                KERNEL_PANIC("Region table overflow");

            size_t region_len = len;
            if (region_len > REGION_MAX_SIZE)
                region_len = REGION_MAX_SIZE;
            start &= MASK_ADDR;
            size_t region_end = (start + region_len);

            region_table_entry_t * region = &region_table->entries[table_index++];
            region->addr_flags = start | REGION_TABLE_FLAG_PRESENT;
            region->page_count = region_len / PAGE_SIZE;
            // -1 for bitmask page
            region->free_count = region->page_count - 1;

            create_bitmask(r);

            len -= region_len;
            start += region_len;
            if (start & MASK_FLAGS)
                start += PAGE_SIZE;
        }
    }
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
    return upper_ram[i].type == RAM_TYPE_USABLE
           || upper_ram[i].type == RAM_TYPE_ACPI_RECLAIMABLE;
}

enum RAM_TYPE ram_upper_type(uint16_t i) {
    upper_ram_t * upper_ram = (upper_ram_t *)UPPER_RAM_ADDR;
    return upper_ram[i].type;
}

uint32_t get_bitmask_addr(size_t i) {
    if (i > REGION_TABLE_SIZE)
        return region_table->entries[i].addr_flags & MASK_ADDR;
}

void * ram_page_alloc() {
    for (size_t i = 0; i < REGION_TABLE_SIZE; i++) {
        uint8_t flag = region_table->entries[i].addr_flags & MASK_FLAGS;
        if (flag & REGION_TABLE_FLAG_PRESENT
            && region_table->entries[i].free_count) {
            uint16_t bit = find_free_bit(i);
            if (!bit)
                KERNEL_PANIC("Could not find free bit in page with free count");

            set_bitmask(i, bit, false);
            region_table->entries[i].free_count--;
            uint32_t page_addr = region_table->entries[i].addr_flags
                                 & MASK_ADDR + bit * PAGE_SIZE;
            return (void *)page_addr;
        }
    }

    return 0;
}

void ram_page_free(void * addr) {
    size_t region_index = find_addr_entry((uint32_t)addr);
    region_table_entry_t * region = &region_table->entries[region_index];
    if (!region)
        return;

    uint16_t bit = find_addr_bit(region_index, (uint32_t)addr);
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

        size_t next_i = i;
        uint64_t next_start = curr_start;

        for (size_t i = i; i < upper_ram_count; i++) {
            if (upper_ram[i].base_addr > next_start) {
                next_i = i;
                next_start = upper_ram[i].base_addr;
            }
        }

        if (next_i != i) {
            swap = upper_ram[i];
            upper_ram[i] = upper_ram[next_i];
            upper_ram[next_i] = upper_ram[i];
        }
    }
}

static void create_bitmask(size_t region_index) {
    if (region_index > REGION_TABLE_SIZE)
        return;

    region_table_entry_t * region = &region_table->entries[region_index];
    uint8_t * bitmask = (uint8_t *)(region->addr_flags & MASK_ADDR);

    size_t pages = region->page_count;
    size_t bytes = pages / 8;
    size_t bits = pages % 8;

    // kprintf("Bitmask at %p for region of %u pages %u bytes and %u bites\n",
    //         bitmask,
    //         pages,
    //         bytes,
    //         bits);

    memset(bitmask, 0, PAGE_SIZE);
    memset(bitmask, 0xff, bytes);

    for (size_t i = 0; i < bits; i++) {
        bitmask[bytes] |= 1 << i;
    }

    // Set bitmask page as used
    set_bitmask(region_index, 0, false);
}

static void set_physical_bitmask(size_t region_index, uint16_t bit, bool free) {
    if (region_index > REGION_TABLE_SIZE)
        return;

    region_table_entry_t * region = &region_table->entries[region_index];
    uint8_t * bitmask = (uint8_t *)(region->addr_flags & MASK_ADDR);
    size_t byte = bit / 8;
    bit = bit % 8;

    if (free) {
        bitmask[byte] |= 1 << bit;
    }
    else {
        bitmask[byte] &= ~(1 << bit);
    }
}

static void set_bitmask(size_t region_index, uint16_t bit, bool free) {
    if (region_index > REGION_TABLE_SIZE)
        return;

    uint8_t * bitmask =
        (uint8_t *)(REGION_VIRT_BITMASK_ADDR + region_index * PAGE_SIZE);
    size_t byte = bit / 8;
    bit = bit % 8;

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

    uint8_t * bitmask =
        (uint8_t *)(REGION_VIRT_BITMASK_ADDR + region_index * PAGE_SIZE);
    size_t byte = bit / 8;
    bit = bit % 8;

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
            uint32_t region_start = region_table->entries[i].addr_flags & MASK_ADDR;
            uint32_t region_end =
                region_start + region_table->entries[i].page_count * PAGE_SIZE;

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
    uint32_t region_end = region_start + region->page_count * PAGE_SIZE;

    if (addr <= region_start || addr >= region_end)
        return 0;

    return (addr - region_start) / PAGE_SIZE;
}
