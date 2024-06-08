#include "drivers/page.h"

#include <stdbool.h>
#include <stddef.h>

#include "drivers/ram.h"
#include "kernel.h"
#include "libc/string.h"

#define PAGE_SIZE 4096
#define REGION_TABLE_SIZE (PAGE_SIZE / sizeof(region_table_entry_t)) // 512
#define REGION_MAX_PAGE 0xffff

#define REGION_TABLE_FLAG_PRESENT 0x1
#define BITMASK_PAGE_FREE 0x1

typedef struct {
    uint32_t addr_flags;
    uint16_t page_count;
    uint16_t free_count;
} __attribute__((packed)) region_table_entry_t;

region_table_entry_t * region_table;

static size_t create_bitmask(uint32_t start, uint32_t end, size_t i);
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
            if (addr & 0xfff)
                addr += PAGE_SIZE;
            region_table = (region_table_entry_t *)(addr & 0xfffff000);
            first_area = i;
            break;
        }
    }

    if (!region_table) {
        KERNEL_PANIC("FAILED TO FIND FREE RAM");
    }

    for (size_t i = 0; i < 512; i++) {
        region_table[i].addr_flags = 0;
        region_table[i].page_count = 0;
        region_table[i].free_count = 0;
    }

    size_t entry_index = 0;
    for (int i = first_area; i < ram_upper_count(); i++) {
        if (!ram_upper_usable(i))
            continue;
        uint32_t start = (uint32_t)ram_upper_start(i);
        uint32_t end = (uint32_t)ram_upper_end(i) & 0xfffff000;

        entry_index = create_bitmask(start, end, entry_index);
    }
}

uint32_t page_alloc() {
    for (size_t i = 0; i < REGION_TABLE_SIZE; i++) {
        uint8_t flag = region_table[i].addr_flags & 0xfff;
        if (flag & REGION_TABLE_FLAG_PRESENT && region_table[i].free_count) {
            uint16_t bit = find_free_bit(&region_table[i]);
            if (!bit)
                KERNEL_PANIC("Could not find free bit in page with free count");

            set_bitmask(&region_table[i], bit, false);
            region_table[i].free_count--;
            return region_table[i].addr_flags & 0xfffff000 + bit * PAGE_SIZE;
        }
    }

    return 0;
}

void page_free(uint32_t addr) {
    region_table_entry_t * region = find_addr_entry(addr);
    if (!region)
        return;

    uint16_t bit = find_addr_bit(region, addr);
    if (!bit)
        return;

    set_bitmask(region, bit, true);
    region->free_count++;
}

static size_t create_bitmask(uint32_t start_addr, uint32_t end_addr, size_t i) {
    if (start_addr % PAGE_SIZE)
        start_addr += PAGE_SIZE;
    start_addr &= 0xfffff000;
    end_addr &= 0xfffff000;

    while (start_addr < end_addr) {
        if (i >= REGION_TABLE_SIZE)
            KERNEL_PANIC("Region table overflow");

        uint8_t * bitmask = (uint8_t *)start_addr;

        size_t len = end_addr - start_addr;
        if (len > REGION_MAX_PAGE * PAGE_SIZE)
            len = REGION_MAX_PAGE * PAGE_SIZE;

        size_t pages = len / PAGE_SIZE;
        size_t bytes = pages / 8;
        size_t bits = pages % 8;

        region_table_entry_t * region = &region_table[i];
        region->addr_flags = start_addr | REGION_TABLE_FLAG_PRESENT;
        region->page_count = pages;
        // -1 to exclude the bitmask page
        region->free_count = pages - 1;

        memset(bitmask, 0, PAGE_SIZE);
        memset(bitmask, 0xff, bytes);

        uint8_t end_mask = 0;
        for (size_t i = 0; i < bits; i++) {
            bits |= 1 << i;
        }

        bitmask[bytes] = end_mask;

        // Set bitmask page as used
        set_bitmask(region, 0, false);

        start_addr += len;
        i++;
    }

    return i;
}

static void set_bitmask(region_table_entry_t * region, uint16_t bit, bool free) {
    uint8_t * bitmask = (uint8_t *)(region->addr_flags & 0xfffff000);
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
    uint8_t * bitmask = (uint8_t *)(region->addr_flags & 0xfffff000);
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
    if (addr & 0xfff)
        return 0;

    for (size_t i = 0; i < REGION_TABLE_SIZE; i++) {
        if (region_table[i].addr_flags & REGION_TABLE_FLAG_PRESENT) {
            uint32_t region_start = region_table[i].addr_flags & 0xfffff000;
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
    if (addr & 0xfff || !(region->addr_flags & REGION_TABLE_FLAG_PRESENT))
        return 0;

    uint32_t region_start = region->addr_flags & 0xfffff000;
    uint32_t region_end = region_start + region->page_count * PAGE_SIZE;

    if (addr <= region_start || addr >= region_end)
        return 0;

    return (addr - region_start) / PAGE_SIZE;
}
