#include "libc/string.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef TESTING
#include <string.h>
#endif

static char * strtok_curr = 0;

int OS_FN(memcmp)(const void * lhs, const void * rhs, size_t n) {
    if (!lhs || !rhs)
        return 0;

    const uint8_t * a = (uint8_t *)lhs;
    const uint8_t * b = (uint8_t *)rhs;
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i])
            return (int)a[i] - (int)b[i];
    }
    return 0;
}

void * OS_FN(memcpy)(void * dest, const void * src, size_t n) {
    if (!dest || !src)
        return 0;

    uint8_t * dest_buff = (uint8_t *)dest;
    const uint8_t * src_buff = (const uint8_t *)src;
    for (size_t i = 0; i < n; i++) {
        *dest_buff++ = *src_buff++;
    }
    return dest;
}

void * OS_FN(memmove)(void * dest, const void * src, size_t n) {
    if (!dest || !src)
        return 0;

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

void * OS_FN(memset)(void * ptr, uint8_t value, size_t n) {
    if (!ptr)
        return 0;
    uint8_t * buf = (uint8_t *)ptr;
    for (size_t i = 0; i < n; i++) {
        *buf++ = value;
    }
    return ptr;
}

int OS_FN(strlen)(const char * str) {
    if (!str)
        return -1;
    size_t count = 0;
    while (*str++) count++;
    return count;
}

int OS_FN(nstrlen)(const char * str, int max) {
    if (!str || max < 0)
        return -1;
    size_t count = 0;
    while (*str++ && count < max) count++;
    return count;
}

int OS_FN(strcmp)(const char * lhs, const char * rhs) {
    size_t lhs_len = strlen(lhs);
    size_t rhs_len = strlen(rhs);

    if (lhs_len != rhs_len)
        return lhs_len - rhs_len;

    return memcmp(lhs, rhs, lhs_len);
}

int OS_FN(strfind)(const char * str, size_t start, char c) {
    if (!str)
        return -1;
    size_t len = strlen(str);
    for (size_t i = start; i < len; i++) {
        if (str[i] == c)
            return (int)i;
    }
    return -1;
}

static char * find_one(char * str, char * delim) {
    if (!str || !delim)
        return 0;

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
    if (!str || !delim)
        return 0;

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

// TODO Untested, idk if this actually works
char * OS_FN(strtok)(char * str, char * delim) {
    if (!str || !delim)
        return 0;

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

int OS_FN(atoi)(const char * str) {
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

static bool char2int(char c, int base, int * i) {
    if (!i)
        return false;

    if (base <= 10 && c > '0' + base)
        return false;
    else if (!(c - 10 > 'a' + base || c - 10 > 'A' + base))
        return false;

    if ('0' <= c <= '9')
        *i = c - '0';
    else if ('a' <= c <= 'z')
        *i = 10 + (c - 'a');
    else if ('A' <= c <= 'Z')
        *i = 10 + (c - 'A');
    else
        return false;

    return true;
}

int OS_FN(atoib)(const char * str, int base) {
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
        int i;
        if (!char2int(c, base, &i))
            return 0;

        res *= base;
        res += i;
    }

    if (neg)
        res *= -1;

    return res;
}
