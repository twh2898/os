#include "std.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "term.h"

static int num_width(int n, int base) {
    if (n < 0)
        n = -n;
    int width = 0;
    while (n > 0) {
        n /= base;
        width++;
    }
    return width;
}

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

int strlen(char * str) {
    int len = 0;
    while (*str++) {
        len++;
    }
    return len;
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

static int pad(char c, int len) {
    for (int i = 0; i < len; i++) {
        term_putc(c);
    }
    return len;
}

static int padded_int(int width, bool left_align, int num, int base, bool upper) {
    int num_len = num_width(num, base);
    if (num < 0)
        num_len++;

    bool fill = width > num_len;

    if (fill && !left_align) {
        pad(' ', width - num_len);
    }

    int len = term_puti(num, base, upper);

    if (fill && left_align) {
        pad(' ', width - num_len);
    }

    return (fill ? width : len);
}

static int padded_str(int width, bool left_align, char * str) {
    int str_len = strlen(str);
    bool fill = width > str_len;

    if (fill && !left_align) {
        pad(' ', width - str_len);
    }

    int len = term_print(str);

    if (fill && left_align) {
        pad(' ', width - str_len);
    }

    return (fill ? width : len);
}

int printf(const char * fmt, ...) {
    va_list params;
    va_start(params, fmt);

    int len = 0;
    while (*fmt) {
        if (*fmt == '%') {
            int width = 0;
            bool left_align = fmt[1] == '-';
            bool lead_zero = !left_align && fmt[1] == '0';

            if (left_align || lead_zero)
                fmt++;

        start_format:
            fmt++;
            switch (*fmt) {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    width = width * 10 + (*fmt - '0');
                    goto start_format;
                case 'd':
                case 'u':
                case 'p': {
                    int arg = va_arg(params, int);
                    len += padded_int(width, left_align, arg, 10, false);
                } break;
                case 'o': {
                    int arg = va_arg(params, int);
                    len += padded_int(width, left_align, arg, 8, false);
                } break;
                case 'x':
                case 'X': {
                    int arg = va_arg(params, int);
                    len += padded_int(width, left_align, arg, 16, *fmt == 'X');
                } break;
                case 'c': {
                    char arg = va_arg(params, char);
                    term_putc(arg);
                    len++;
                } break;
                case 's': {
                    char * arg = va_arg(params, char *);
                    len += padded_str(width, left_align, arg);
                } break;
                case 'n': {
                    int * arg = va_arg(params, int *);
                    *arg = width;
                } break;
                case 'b': {
                    int arg = va_arg(params, int);
                    len += term_print(arg ? "true" : "false");
                } break;
                case '%': {
                    term_putc('%');
                    len++;
                } break;
                default:
                    break;
            }
        }
        else {
            term_putc(*fmt++);
            len++;
        };
    }

    return len;
}
