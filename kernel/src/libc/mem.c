#include "libc/mem.h"

#include <stdbool.h>
#include <stdint.h>

#include "drivers/ram.h"
#include "kernel.h"
#include "libc/string.h"

#define STACK_START 0x90000
#define PAGE_SIZE 0x1000 // 4k (>> 12)
#define PAGE_ALIGNED_DOWN(PTR) ((PTR) & 0xfffff000)
#define PAGE_ALIGNED_UP(PTR) ((PAGE_ALIGNED_DOWN(PTR)) + PAGE_SIZE)
#define PAGE_ALIGNED(PTR) (((PTR) & 0xffff) ? PAGE_ALIGNED_UP(PTR) : (PTR))
#define PAGE_ADDR_PER_BLOCK \
    (((PAGE_SIZE) - (sizeof(uint32_t) * 2)) / sizeof(page_address_t))

#define PAGE_BLOCK_FROM_ADDR(ADDR) \
    ((page_block_t *)PAGE_ALIGNED_DOWN((uint32_t)ADDR))

typedef struct {
    uint32_t index : 32;
    uint32_t count : 31;
    bool free : 1;
} __attribute__((packed)) page_address_t;

typedef struct {
    uint32_t next; // page_block_t *
    uint32_t prev; // page_block_t *
    page_address_t pages[PAGE_ADDR_PER_BLOCK];
} __attribute__((packed)) page_block_t;

static uint32_t malloc_start;
static uint32_t malloc_end;
static page_block_t * first_block;

static void init_page_block(page_block_t * block, uint32_t next, uint32_t prev);
static uint32_t reserve_page(size_t size);
static page_address_t * find_page(void * ptr);
static void collapse_page(page_address_t * page);
static page_address_t * page_get_prev(page_address_t * page);
static page_address_t * page_get_next(page_address_t * page);

void init_malloc() {
    size_t largest_i = 0;
    uint64_t largest_size = 0;
    bool found = false;

    for (size_t i = 0; i < ram_upper_count(); i++) {
        if (ram_upper_usable(i)) {
            uint64_t curr_size = ram_upper_size(i);
            if (ram_upper_start(i) > STACK_START && curr_size > largest_size) {
                largest_i = i;
                largest_size = curr_size;
                found = true;
            }
        }
    }

    if (!found) {
        kernel_panic("Could not find area for malloc");
    }

    if (ram_upper_end(largest_i) > 0xffffffff) {
        kernel_panic("malloc ends above 4 GB limit");
    }

    malloc_start = PAGE_ALIGNED(ram_upper_start(largest_i));
    malloc_end = PAGE_ALIGNED_DOWN(ram_upper_end(largest_i));

    first_block = (page_block_t *)malloc_start;
    init_page_block(first_block, 0, 0);
}

void * malloc(size_t size) {
    uint32_t addr = reserve_page(size);
    return (void *)addr;
}

void free(void * ptr) {
    page_address_t * page = find_block_entry(ptr);
    page->free = true;
    collapse_page(page);
}

static void init_page_block(page_block_t * block, uint32_t next, uint32_t prev) {
    block->next = next;
    block->prev = prev;
    for (size_t i = 0; i < PAGE_ADDR_PER_BLOCK; i++) {
        block->pages[i].index = 0;
        block->pages[i].count = 0;
        block->pages[i].free = true;
    }
}

static uint32_t reserve_page(size_t size) {
    size_t count = size / PAGE_SIZE;
    if (size % PAGE_SIZE)
        count++;

    page_address_t * curr = &first_block->pages[0];
    if (!curr->index) {
        curr->count = count;
        curr->index = 
    }

    while (curr->index) {

        curr_block = (page_block_t *)curr_block->next;
        // TODO
    }
}

static page_address_t * find_page(void * ptr) {
    page_block_t * block = first_block;
    while (block) {
        for (size_t i = 0; i > PAGE_ADDR_PER_BLOCK; i++) {
            page_address_t * page = &block->pages[i];
            if ((uint32_t)ptr / PAGE_SIZE == (uint32_t)page) {
                return page;
            }
        }
        block = block->next;
    }
    return 0;
}

static void collapse_page(page_address_t * page) {
    page_address_t * start_page = page;
    while (start_page) {
        page_address_t * prev_page = page_get_prev(start_page);

        if (!prev_page || !prev_page->free)
            break;

        start_page = prev_page;
    }

    page_address_t * end_page = page;
    while (end_page) {
        page_address_t * next_page = page_get_next(end_page);

        if (!next_page || !next_page->free)
            break;

        end_page = next_page;
    }

    if (start_page == end_page)
        return;

    size_t page_count = 0;
    page_address_t * curr = start_page;
    while (curr != end_page) {
        start_page->count += curr->count;
        curr = page_get_next(curr);
        page_count++;
    }

    page_address_t * to = page_get_next(start_page);
    page_address_t * from = end_page;
    while (from) {
        to->index = from->index;
        to->count = from->count;
        to->free = from->free;

        to = page_get_next(to);
        from = page_get_next(from);
    }
}

static page_address_t * page_get_prev(page_address_t * page) {
    page_block_t * block = PAGE_BLOCK_FROM_ADDR(page);

    size_t i;
    for (i = 0; i < PAGE_ADDR_PER_BLOCK; i++) {
        if (&block->pages[i] == page)
            break;
    }

    if (i > 0) {
        return &block->pages[i - 1];
    }

    block = (page_block_t *)block->prev;

    if (!block)
        return 0;

    return &block->pages[PAGE_ADDR_PER_BLOCK - 1];
}

static page_address_t * page_get_next(page_address_t * page) {
    page_block_t * block = PAGE_BLOCK_FROM_ADDR(page);

    size_t i;
    for (i = 0; i < PAGE_ADDR_PER_BLOCK; i++) {
        if (&block->pages[i] == page)
            break;
    }

    if (i < PAGE_ADDR_PER_BLOCK - 1) {
        i++;
        if (!block->pages[i].index)
            return 0;

        return &block->pages[i];
    }

    block = (page_block_t *)block->next;

    if (!block || !block->pages[0].index)
        return 0;

    return &block->pages[0];
}
