#include "libc/memory.h"

#include "libc/string.h"
#include "libk/sys_call.h"
#include "memory_alloc.h"

static memory_t __memory = {.first = 0, .last = 0};

static void init() {
    memory_init(&__memory, _page_alloc);
}

void * kmalloc(size_t size) {
    if (!__memory.first) {
        init();
    }

    return memory_alloc(&__memory, size);
}

void * krealloc(void * ptr, size_t size) {
    if (!__memory.first) {
        init();
    }

    return memory_realloc(&__memory, ptr, size);
}

void kfree(void * ptr) {
    memory_free(&__memory, ptr);
}
