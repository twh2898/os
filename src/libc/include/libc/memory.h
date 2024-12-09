#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

void * malloc(size_t size);
void * calloc(size_t size, uint8_t value);
void * realloc(void * ptr, size_t size);
void free(void * ptr);

#endif // MEMORY_H
