#include "libk/sys_call.h"

void * _malloc(size_t size) {
    return 0;
}

void * _realloc(void * ptr, size_t size) {
    return 0;
}

void _free(void * ptr) {
}

// void _exit(uint8_t code) {
// }

size_t _putc(char c) {
    return 0;
}

size_t _puts(const char * str) {
    return 0;
}
