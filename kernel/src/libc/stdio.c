#include "libc/stdio.h"

#include <stdarg.h>
#include <stdint.h>

#include "drivers/vga.h"
#include "libc/string.h"

static int num_width(unsigned int n, int base) {
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

size_t puts(const char * str) {
    return vga_print(str);
}

size_t putc(char c) {
    return vga_putc(c);
}

static char digit(unsigned int num, int base, bool upper) {
    if (num < 10)
        return num + '0';
    else
        return (num - 10) + (upper ? 'A' : 'a');
}

size_t puti(int num, int base, bool upper) {
    if (num == 0) {
        return vga_putc('0');
    }

    bool is_neg = num < 0;

    size_t o_len = 0;
    if (num < 0) {
        o_len += vga_putc('-');
        num = -num;
    }

    size_t len = 0;
    int rev = 0;
    while (num > 0) {
        rev = (rev * base) + (num % base);
        num /= base;
        len++;
    }

    for (size_t i = 0; i < len; i++) {
        o_len += vga_putc(digit(rev % base, base, upper));
        rev /= base;
    }

    return o_len;
}


size_t putu(unsigned int num, unsigned int base, bool upper) {
    if (num == 0) {
        return vga_putc('0');
    }

    size_t len = 0;
    int rev = 0;
    while (num > 0) {
        rev = (rev * base) + (num % base);
        num /= base;
        len++;
    }

    size_t o_len = 0;
    for (size_t i = 0; i < len; i++) {
        o_len += vga_putc(digit(rev % base, base, upper));
        rev /= base;
    }

    return o_len;
}

static size_t pad(char c, size_t len) {
    size_t o_len = 0;
    for (size_t i = 0; i < len; i++) {
        o_len += vga_putc(c);
    }
    return o_len;
}

static size_t padded_int(
    int width, bool left_align, int num, int base, bool upper, bool lead_zero) {
    int num_len = num_width(num, base);
    bool is_neg = num < 0;

    if (is_neg) {
        num_len++;
        num = -num;
    }

    bool fill = width > num_len;

    size_t o_len = 0;
    if (fill && !left_align) {
        if (lead_zero && is_neg) {
            o_len += vga_putc('-');
        }
        o_len += pad((lead_zero ? '0' : ' '), width - num_len);
        if (!lead_zero && is_neg) {
            o_len += vga_putc('-');
        }
    }
    else if (is_neg) {
        o_len += vga_putc('-');
    }

    o_len += puti(num, base, upper);

    if (fill && left_align) {
        o_len += pad(' ', width - num_len);
    }

    return o_len;
}

static size_t padded_uint(int width,
                          bool left_align,
                          unsigned int num,
                          int base,
                          bool upper,
                          bool lead_zero) {
    int num_len = num_width(num, base);

    bool fill = width > num_len;

    size_t o_len = 0;
    if (fill && !left_align) {
        o_len += pad((lead_zero ? '0' : ' '), width - num_len);
    }

    o_len += putu(num, base, upper);

    if (fill && left_align) {
        o_len += pad(' ', width - num_len);
    }

    return o_len;
}

static size_t padded_str(int width, bool left_align, char * str) {
    int str_len = strlen(str);
    bool fill = width > str_len;

    size_t o_len = 0;
    if (fill && !left_align) {
        o_len += pad(' ', width - str_len);
    }

    int len = vga_print(str);

    if (fill && left_align) {
        o_len += pad(' ', width - str_len);
    }

    return o_len;
}

size_t printf(const char * fmt, ...) {
    va_list params;
    va_start(params, fmt);

    size_t o_len = 0;
    while (*fmt) {
        if (*fmt == '%') {
            int width = 0;
            int fract = 0;
            bool fill_fract = false;
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
                    if (!fill_fract)
                        width = width * 10 + (*fmt - '0');
                    else
                        fract = fract * 10 + (*fmt - '0');
                    goto start_format;
                case '.':
                    fill_fract = true;
                    goto start_format;
                case 'd': {
                    int arg = va_arg(params, int);
                    o_len +=
                        padded_int(width, left_align, arg, 10, false, lead_zero);
                } break;
                case 'u': {
                    unsigned int arg = va_arg(params, unsigned int);
                    o_len +=
                        padded_uint(width, left_align, arg, 10, false, lead_zero);
                } break;
                case 'p': {
                    unsigned int arg = va_arg(params, unsigned int);
                    o_len += puts("0x");
                    o_len += padded_uint(width, left_align, arg, 16, false, true);
                } break;
                case 'o': {
                    int arg = va_arg(params, int);
                    o_len +=
                        padded_int(width, left_align, arg, 8, false, lead_zero);
                } break;
                case 'x':
                case 'X': {
                    int arg = va_arg(params, int);
                    o_len += padded_uint(
                        width, left_align, arg, 16, *fmt == 'X', lead_zero);
                } break;
                case 'c': {
                    char arg = va_arg(params, int);
                    o_len += vga_putc(arg);
                } break;
                case 's': {
                    char * arg = va_arg(params, char *);
                    o_len += padded_str(width, left_align, arg);
                } break;
                case 'n': {
                    int * arg = va_arg(params, int *);
                    *arg = width;
                } break;
                case 'b': {
                    int arg = va_arg(params, int);
                    o_len += vga_print(arg ? "true" : "false");
                } break;
                case 'f': {
                    float arg = va_arg(params, double);
                    int lhs = (int)arg;
                    size_t count = puti(lhs, 10, false);
                    o_len += count;
                    o_len += putc('.');
                    float rem = arg - (float)lhs;
                    if (!fract)
                        fract = 6;
                    size_t f_count = 0;
                    while ((!width || count++ < width) && f_count++ < fract) {
                        rem *= 10;
                        // if (rem == 0)
                        //     break;
                        putu((int)rem, 10, false);
                        rem -= (int)rem;
                    }
                } break;
                case '%': {
                    o_len += vga_putc('%');
                } break;
                default:
                    break;
            }
            fmt++;
        }
        else {
            o_len += vga_putc(*fmt++);
        };
    }

    return o_len;
}
