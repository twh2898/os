#ifndef MEMORY_H
#define MEMORY_H

#include "cpu/mmu.h"
#include "defs.h"

void init_malloc(mmu_page_dir_t * dir, size_t first_page);

void * kmalloc(size_t size);
void   kfree(void * ptr);

#endif // MEMORY_H
