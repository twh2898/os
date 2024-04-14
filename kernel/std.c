#include "std.h"

#include <stdint.h>

void itoa(int n, char * str) {
    if (n < 0) {
        *str++ = '-';
        n = -n;
    }

    int len = 0;
    int rev = 0;
    while (n > 0) {
        rev = (rev * 10) + (n % 10);
        n /= 10;
        len += 1;
    }

    for (int i = 0; i < len; i++) {
        *str++ = '0' + (rev % 10);
        rev /= 10;
    }

    if (len == 0) {
        *str++ = '0';
    }

    *str = 0;
}

void * memmove(void * destptr, const void * srcptr, int n) {
    uint8_t * dest = (uint8_t *)destptr;
    const uint8_t * src = (uint8_t *)srcptr;
    int i;
    if (dest < src) {
        for (i = 0; i < n; i++, dest++, src++) *dest = *src;
    }
    else {
        dest = dest + n;
        src = src + n;
        for (i = 0; i < n; i++, dest--, src--) *dest = *src;
    }
    return destptr;
}
