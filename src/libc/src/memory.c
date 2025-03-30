#include "libc/memory.h"

#include "libk/sys_call.h"

void * pmalloc(size_t size) {
    return _sys_mem_malloc(size);
}

void * prealloc(void * ptr, size_t size) {
    return _sys_mem_realloc(ptr, size);
}

void pfree(void * ptr) {
    _sys_mem_free(ptr);
}
