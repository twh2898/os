#include "block.h"

#include "libc/string.h"

void block_init(block_t * block, block_t * prev) {
    block->prev = prev;
    block->next = 0;

    if (block->prev) {
        block->prev->next = block;
    }

    memset(block->pages, 0, sizeof(block->pages));
}

page_t * block_page(const block_t * block, size_t index) {
    if (index >= N_PAGES)
        return 0;

    return &block->pages[index];
}
