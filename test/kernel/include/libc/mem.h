#ifndef MEM_H
#define MEM_H

#include <stddef.h>

/* At this stage there is no 'free' implemented. */
void * malloc(size_t size);

void * realloc(void * ptr, size_t size);
void free(void * ptr);

#endif // MEM_H
