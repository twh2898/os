#include "libc/memory.h"

#include "libc/string.h"
#include "libk/sys_call.h"

void * malloc(size_t size) {
    return _malloc(size);
}

void * calloc(size_t size, uint8_t value) {
    void * ptr = _malloc(size);
    memset(ptr, value, size);
    return ptr;
}

void * realloc(void * ptr, size_t size) {
    return _realloc(ptr, size);
}

void free(void * ptr) {
    _free(ptr);
}
