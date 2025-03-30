#ifndef LIBC_MEMORY_H
#define LIBC_MEMORY_H

#include <stddef.h>
#include <stdint.h>

void * pmalloc(size_t size);
void * prealloc(void * ptr, size_t size);
void   pfree(void * ptr);

#endif // LIBC_MEMORY_H
