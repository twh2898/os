#ifndef BLOCK_H
#define BLOCK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "page.h"

#define BLOCK_HEADER_SIZE (sizeof(page_t *) * 2)
#define N_PAGES ((PAGE_SIZE - BLOCK_HEADER_SIZE) / sizeof(page_t))

typedef struct block_ {
    struct block_ * prev;
    struct block_ * next;
    page_t pages[N_PAGES];
} __attribute__((packed)) block_t;

void block_init(block_t * block, block_t * prev);

page_t * block_page(block_t * block, size_t index);

/* TODO

- Find page from pointer
- Find next block of at least size
- Merge blocks on free
- Generate new block when needed
*/

#endif // BLOCK_H