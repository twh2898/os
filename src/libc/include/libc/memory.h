#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

#ifdef TESTING
#define OS_FN(func) OS_##func
#else
#define OS_FN(func) func
#endif

void * OS_FN(malloc)(size_t size);
void * OS_FN(calloc)(size_t size, uint8_t value);
void * OS_FN(realloc)(void * ptr, size_t size);
void   OS_FN(free)(void * ptr);

#endif // MEMORY_H
