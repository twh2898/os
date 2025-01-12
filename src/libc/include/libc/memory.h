#ifndef LIBC_MEMORY_H
#define LIBC_MEMORY_H

#include <stddef.h>
#include <stdint.h>

void * kmalloc(size_t size);
void * krealloc(void * ptr, size_t size);
void   kfree(void * ptr);

#endif // LIBC_MEMORY_H
