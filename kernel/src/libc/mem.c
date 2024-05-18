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

typedef struct {
    uint32_t page_addr : 32;
    uint32_t page_count : 31;
    bool free : 1;
} __attribute__((packed)) page_address_t;

typedef struct {
    uint32_t next_page_index;
    uint32_t last_page_index;
    page_address_t pages[PAGE_ADDR_PER_BLOCK];
} __attribute__((packed)) page_block_t;

static uint32_t malloc_start;
static uint32_t malloc_end;
static page_block_t * first_block;

static void init_page_block(page_block_t * block, uint32_t next, uint32_t prev);

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

    first_block = malloc_start;
    init_page_block(first_block, 0, 0);
}

void * malloc(size_t size) {
    page_block_t * curr_block = first_block;
    for (size_t i = 0; i < PAGE_ADDR_PER_BLOCK; i++) {
        if (!curr_block->pages[i].free)
            continue;

        if (curr_block->pages[i].size)
    }
    uint32_t ret = free_mem_addr;
    free_mem_addr = PAGE_ALIGNED(free_mem_addr + size);
    return (void *)ret;
}

void * realloc(void * ptr, size_t new_size) {
    // if size is smaller return same???
}

void free(void * ptr) {
    // size = 0
}

static void init_page_block(page_block_t * block, uint32_t next, uint32_t prev) {
    block->last_page_index = prev;
    block->next_page_index = next;
    for (size_t i = 0; i < PAGE_ADDR_PER_BLOCK; i++) {
        block->pages[i].page_addr = 0;
        block->pages[i].page_count = 0;
        block->pages[i].free = true;
    }
}
