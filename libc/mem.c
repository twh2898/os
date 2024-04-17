#include "mem.h"

/* This should be computed at link time, but a hardcoded
 * value is fine for now. Remember that our kernel starts
 * at 0x1000 as defined on the Makefile */
uint32_t free_mem_addr = 0x10000;

/* Implementation is just a pointer to some free memory which
 * keeps growing */
void * malloc(size_t size) {
    /* Pages are aligned to 4K, or 0x1000 */
    if (free_mem_addr & 0xFFFFF000) {
        free_mem_addr &= 0xFFFFF000;
        free_mem_addr += 0x1000;
    }

    uint32_t ret = free_mem_addr;
    free_mem_addr += size; /* Remember to increment the pointer */
    return (void *)ret;
}
