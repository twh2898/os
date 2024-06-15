#include "drivers/page.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "defs.h"
#include "drivers/ram.h"
#include "kernel.h"
#include "libc/stdio.h"
#include "libc/string.h"

#define REGION_TABLE_SIZE (PAGE_SIZE / sizeof(region_table_entry_t)) // 512
#define REGION_MAX_PAGE_COUNT 0x8000
#define REGION_MAX_SIZE (REGION_MAX_PAGE_COUNT * PAGE_SIZE)

#define REGION_TABLE_FLAG_PRESENT 0x1
#define BITMASK_PAGE_FREE 0x1

typedef struct {
    uint32_t addr_flags;
    uint16_t page_count;
    uint16_t free_count;
} __attribute__((packed)) region_table_entry_t;

region_table_entry_t * region_table;

static void create_bitmask(region_table_entry_t * region);
static void set_bitmask(region_table_entry_t * region, uint16_t bit, bool free);
static bool get_bitmask(region_table_entry_t * region, uint16_t bit);
// Returns 0 for error, 0 is always invalid
static uint16_t find_free_bit(region_table_entry_t * region);
static region_table_entry_t * find_addr_entry(uint32_t addr);
// Returns 0 for error, 0 is always invalid
static size_t find_addr_bit(region_table_entry_t * region, uint32_t addr);

void init_pages() {
    size_t first_area = 0;

    for (size_t i = 0; i < ram_upper_count(); i++) {
        uint32_t addr = (uint32_t)ram_upper_start(i);
        if (ram_upper_usable(i) && addr > 0x9fbff) {
            if (addr & MASK_FLAGS)
                addr += PAGE_SIZE;
            region_table = (region_table_entry_t *)(addr & MASK_ADDR);
            first_area = i;
            break;
        }
    }

    if (!region_table) {
        KERNEL_PANIC("FAILED TO FIND FREE RAM");
    }

    // kprintf("Found available at index %u of %u\n", first_area,
    // ram_upper_count()); kprintf("Region table at %p\n", region_table);

    memset(region_table, 0, sizeof(region_table_entry_t) * 512);

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

            region_table_entry_t * region = &region_table[table_index++];
            region->addr_flags = start | REGION_TABLE_FLAG_PRESENT;
            region->page_count = region_len / PAGE_SIZE;
            // -1 for bitmask page
            region->free_count = region->page_count - 1;

            create_bitmask(region);

            len -= region_len;
            start += region_len;
            if (start & MASK_FLAGS)
                start += PAGE_SIZE;
        }
    }
}

void * page_alloc() {
    for (size_t i = 0; i < REGION_TABLE_SIZE; i++) {
        uint8_t flag = region_table[i].addr_flags & MASK_FLAGS;
        if (flag & REGION_TABLE_FLAG_PRESENT && region_table[i].free_count) {
            uint16_t bit = find_free_bit(&region_table[i]);
            if (!bit)
                KERNEL_PANIC("Could not find free bit in page with free count");

            set_bitmask(&region_table[i], bit, false);
            region_table[i].free_count--;
            uint32_t page_addr =
                region_table[i].addr_flags & MASK_ADDR + bit * PAGE_SIZE;
            return (void *)page_addr;
        }
    }

    return 0;
}

void page_free(void * addr) {
    region_table_entry_t * region = find_addr_entry((uint32_t)addr);
    if (!region)
        return;

    uint16_t bit = find_addr_bit(region, (uint32_t)addr);
    if (!bit)
        return;

    set_bitmask(region, bit, true);
    region->free_count++;
}

static void create_bitmask(region_table_entry_t * region) {
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
    set_bitmask(region, 0, false);
}

static void set_bitmask(region_table_entry_t * region, uint16_t bit, bool free) {
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

static bool get_bitmask(region_table_entry_t * region, uint16_t bit) {
    uint8_t * bitmask = (uint8_t *)(region->addr_flags & MASK_ADDR);
    size_t byte = bit / 8;
    bit = bit % 8;

    // Clamp true to 1
    return (bitmask[byte] & (1 << bit)) >> bit;
}

// Returns 0 on failure, 0 is always invalid
static uint16_t find_free_bit(region_table_entry_t * region) {
    if (region->free_count == 0)
        return 0;

    for (uint16_t i = 1; i < region->page_count; i++) {
        if (get_bitmask(region, i))
            return i;
    }

    return 0;
}

static region_table_entry_t * find_addr_entry(uint32_t addr) {
    if (addr & MASK_FLAGS)
        return 0;

    for (size_t i = 0; i < REGION_TABLE_SIZE; i++) {
        if (region_table[i].addr_flags & REGION_TABLE_FLAG_PRESENT) {
            uint32_t region_start = region_table[i].addr_flags & MASK_ADDR;
            uint32_t region_end =
                region_start + region_table[i].page_count * PAGE_SIZE;

            if (addr >= region_start && addr <= region_end)
                &region_table[i];
        }
    }

    return 0;
}

// Returns 0 for error, 0 is always invalid
static size_t find_addr_bit(region_table_entry_t * region, uint32_t addr) {
    if (addr & MASK_FLAGS || !(region->addr_flags & REGION_TABLE_FLAG_PRESENT))
        return 0;

    uint32_t region_start = region->addr_flags & MASK_ADDR;
    uint32_t region_end = region_start + region->page_count * PAGE_SIZE;

    if (addr <= region_start || addr >= region_end)
        return 0;

    return (addr - region_start) / PAGE_SIZE;
}
