#include "libc/memory.h"

#include "libc/string.h"
#include "libk/sys_call.h"

static memory_t * __memory;

void init_malloc(memory_t * memory) {
    __memory = memory;
}

void * kmalloc(size_t size) {
    return memory_alloc(__memory, size);
}

void * krealloc(void * ptr, size_t size) {
    return memory_realloc(__memory, ptr, size);
}

void kfree(void * ptr) {
    memory_free(__memory, ptr);
}
