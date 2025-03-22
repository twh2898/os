#ifndef KERNEL_KMEM_H
#define KERNEL_KMEM_H

#include <stddef.h>
#include <stdint.h>

#include "memory_alloc.h"

void * kmalloc(size_t size);
void * krealloc(void * ptr, size_t size);
void   kfree(void * ptr);

#endif // KERNEL_KMEM_H
