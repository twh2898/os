#include "libc/memory.h"

#include "libc/string.h"
#include "libk/sys_call.h"

static memory_t * __memory;

void init_malloc(memory_t * memory) {
    __memory = memory;
}

void * pmalloc(size_t size) {
    return memory_alloc(__memory, size);
}

void * prealloc(void * ptr, size_t size) {
    return memory_realloc(__memory, ptr, size);
}

void pfree(void * ptr) {
    memory_free(__memory, ptr);
}
