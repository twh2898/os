#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

void * kmalloc(size_t size);
void * kcalloc(size_t size, uint8_t value);
void * krealloc(void * ptr, size_t size);
void   kfree(void * ptr);

#endif // MEMORY_H
