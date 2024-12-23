#ifndef MEMORY_H
#define MEMORY_H

#include "cpu/mmu.h"
#include "defs.h"

void init_malloc(mmu_page_dir_t * dir, size_t first_page);

void * impl_kmalloc(size_t size);
void   impl_kfree(void * ptr);

#endif // MEMORY_H
