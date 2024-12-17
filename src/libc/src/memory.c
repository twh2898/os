#include "libc/memory.h"

#include "libc/string.h"
#include "libk/sys_call.h"

void * kmalloc(size_t size) {
    return _malloc(size);
}

void * kcalloc(size_t size, uint8_t value) {
    void * ptr = _malloc(size);
    kmemset(ptr, value, size);
    return ptr;
}

void * krealloc(void * ptr, size_t size) {
    return _realloc(ptr, size);
}

void kfree(void * ptr) {
    _free(ptr);
}
