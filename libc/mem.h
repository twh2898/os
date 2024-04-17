#ifndef MEM_H
#define MEM_H

#include <stddef.h>
#include <stdint.h>

/* At this stage there is no 'free' implemented. */
void * malloc(size_t size);

#endif // MEM_H