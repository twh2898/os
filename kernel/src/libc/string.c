#include "libc/string.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "term.h"

int memcmp(const void * lhs, const void * rhs, size_t n) {
    const uint8_t * a = (uint8_t *)lhs;
    const uint8_t * b = (uint8_t *)rhs;
    for (size_t i = 0; i < n; i++, a++, b++) {
        if (*a != *b)
            return (int)*a - (int)*b;
    }
    return 0;
}

void * memcpy(void * dest, const void * src, size_t n) {
    uint8_t * dest_buff = (uint8_t *)dest;
    const uint8_t * src_buff = (const uint8_t *)src;
    for (size_t i = 0; i < n; i++) {
        *dest_buff++ = *src_buff++;
    }
    return dest;
}

void * memmove(void * dest, const void * src, size_t n) {
    uint8_t * dest_buff = (uint8_t *)dest;
    const uint8_t * src_buff = (const uint8_t *)src;
    if (dest_buff < src_buff) {
        for (size_t i = 0; i < n; i++) {
            *dest_buff++ = *src_buff++;
        }
    }
    else {
        dest_buff = dest_buff + n;
        src = src + n;
        for (size_t i = 0; i < n; i++) {
            *dest_buff-- = *src_buff--;
        }
    }
    return dest;
}

void * memset(void * ptr, uint8_t value, size_t n) {
    uint8_t * buf = (uint8_t *)ptr;
    for (size_t i = 0; i < n; i++) {
        *buf++ = value;
    }
    return ptr;
}

size_t strlen(const char * str) {
    size_t count = 0;
    while (*str++) count++;
    return count;
}
