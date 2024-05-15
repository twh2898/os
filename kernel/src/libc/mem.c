#include "libc/mem.h"

#include <stdbool.h>
#include <stdint.h>

#include "drivers/ram.h"
#include "kernel.h"
#include "libc/string.h"

#define STACK_START 0x90000

#include "libc/intern/block.h"
#include "libc/intern/page.h"

uint32_t malloc_start;
uint32_t malloc_end;

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

    malloc_start = ram_upper_start(largest_i);
    malloc_end = ram_upper_end(largest_i);
}

// typedef struct _page_t {
//     struct _page_t * next;
//     struct _page_t * prev;
//     entry_t entries[ENTRIES_PER_PAGE];
// } __attribute__((packed)) page_t;

// page_t * first_page = 0;

// bool entry_is_free(entry_t * entry) {
//     return entry->size == 0;
// }

// void page_init(page_t * page, page_t * prev) {
//     page->next = 0;
//     page->prev = prev;
//     if (prev) {
//         prev->next = page;
//     }
//     page->entries[0].start = page += PAGE_SIZE;
//     page->entries[0].size = 0;
//     for (size_t i = 1; i < ENTRIES_PER_PAGE; i++) {
//         page->entries[i].start = 0;
//         page->entries[i].size = 0;
//     }
// }

// static uint32_t aligned(uint32_t ptr) {
//     /* Pages are aligned to 4K, or 0x1000 */
//     if (ptr & 0xFFFFF000) {
//         ptr &= 0xFFFFF000;
//         ptr += PAGE_SIZE;
//     }
//     return ptr;
// }

// entry_t * page_next_free(size_t size) {
//     page_t * page = first_page;
//     for (int i = 0; i < ENTRIES_PER_PAGE; i++) {
//         // page->entries[0].start should never be 0
//         if (i > 0 && page->entries[i].start == 0) {
//             page->entries[i].start = page_aligned(page->entries[i - 1].start
//                                                   + page->entries[i -
//                                                   1].size);
//         }
//         else if (entry_is_free(&page->entries[i]) && page->entries[i].size) {
//         }
//     }
// }

// void page_elem_new(size_t size) {
//     if (!first_page) {
//         first_page = PAGE_START;
//         page_init(first_page, 0);
//     }

//     entry_t * next = elem_next_free(size);
// }

// create
// find ptr
// free
// find free of size
// shift all

// TODO PAGE API

/* This should be computed at link time, but a hardcoded
 * value is fine for now. Remember that our kernel starts
 * at 0x1000 as defined on the Makefile */
uint32_t free_mem_addr = PAGE_ALIGNED(PAGE_START);

static void init_block(page_t * block) {
    // page_t * page = &block_start[0];
    // page->prev = 0;
    // page->next = block_start + sizeof(page_t);
    // page->start = free_mem_addr;
    // page->size = 0;

    // for (size_t i = 1; i < ENTRIES_PER_PAGE; i++) {
    //     page_t * page = block_start + (i * sizeof(page_t));
    //     page->prev = page -= sizeof(page_t);

    //     if (i < ENTRIES_PER_PAGE - 1) {
    //         page->next = page += sizeof(page_t);
    //     }
    //     else {
    //         page->next = 0;
    //     }
    // }
}

// static void extend() {
//     page_t * page = block_start;
//     while (page->next != 0) {
//         page = page->next;
//     }
// }

// static uint32_t aligned(uint32_t ptr) {
//     /* Pages are aligned to 4K, or 0x1000 */
//     if (ptr & 0xFFFFF000) {
//         ptr &= 0xFFFFF000;
//         ptr += PAGE_SIZE;
//     }
//     return ptr;
// }

/* Implementation is just a pointer to some free memory which
 * keeps growing */
void * malloc(size_t size) {
    uint32_t ret = free_mem_addr;
    free_mem_addr += size; /* Remember to increment the pointer */
    return (void *)ret;
}

void * realloc(void * ptr, size_t new_size) {
    // if size is smaller return same???
}

void free(void * ptr) {
    // size = 0
}
