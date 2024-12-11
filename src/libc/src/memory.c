#include "libc/memory.h"

#ifdef TESTING
#include <stdlib.h>
#include <string.h>
#else
#include "libc/string.h"
#endif

#include "libk/sys_call.h"

void * OS_FN(malloc)(size_t size) {
    return _malloc(size);
}

void * OS_FN(calloc)(size_t size, uint8_t value) {
    void * ptr = _malloc(size);
    memset(ptr, value, size);
    return ptr;
}

void * OS_FN(realloc)(void * ptr, size_t size) {
    return _realloc(ptr, size);
}

void OS_FN(free)(void * ptr) {
    _free(ptr);
}
