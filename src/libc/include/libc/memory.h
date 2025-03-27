#ifndef LIBC_MEMORY_H
#define LIBC_MEMORY_H

#include <stddef.h>
#include <stdint.h>

#include "memory_alloc.h"

void   init_malloc(memory_t * memory);
void * pmalloc(size_t size);
void * prealloc(void * ptr, size_t size);
void   pfree(void * ptr);

#endif // LIBC_MEMORY_H
