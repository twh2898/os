#include "libc/string.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "term.h"

static char * strtok_curr = 0;

int memcmp(const void * lhs, const void * rhs, size_t n) {
    const uint8_t * a = (uint8_t *)lhs;
    const uint8_t * b = (uint8_t *)rhs;
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i])
            return (int)a[i] - (int)b[i];
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

int strfind(const char * str, size_t start, char c) {
    size_t len = strlen(str);
    for (size_t i = start; i < len; i++) {
        if (str[i] == c)
            return (int)i;
    }
    return -1;
}

static char * find_one(char * str, char * delim) {
    size_t len = strlen(str);
    size_t d_len = strlen(delim);

    for (size_t i = 0; i < len; i++) {
        for (size_t d = 0; d < d_len; d++) {
            if (str[i] == delim[d]) {
                return str + i;
            }
        }
    }
    return str;
}

static char * find_not_one(char * str, char * delim) {
    size_t len = strlen(str);
    size_t d_len = strlen(delim);

    for (size_t i = 0; i < len; i++) {
        for (size_t d = 0; d < d_len; d++) {
            if (str[i] != delim[d]) {
                return str + i;
            }
        }
    }
    return str;
}

// FIXME: Untested, idk if this actually works
char * strtok(char * str, char * delim) {
    if (str)
        strtok_curr = str;

    if (strtok_curr == 0)
        return 0;

    strtok_curr = find_not_one(strtok_curr, delim);
    size_t len = strlen(strtok_curr);
    if (len == 0)
        return 0;

    str = strtok_curr;

    strtok_curr = find_one(strtok_curr, delim);
    if (strtok_curr == str) {
        strtok_curr += len;
    }

    *strtok_curr = 0;
    return str;
}

int atoi(const char * str) {
    if (!str)
        return 0;

    bool neg = false;
    if (*str == '+')
        str++;
    else if (*str == '-') {
        neg = true;
        str++;
    }

    int res = 0;
    while (*str) {
        char c = *str++;
        if (c < '0' || c > '9')
            return 0;

        res *= 10;
        res += (c - '0');
    }

    if (neg)
        res *= -1;

    return res;
}
