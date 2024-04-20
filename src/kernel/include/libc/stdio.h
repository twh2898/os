#ifndef STDIO_H
#define STDIO_H

#include <stdbool.h>
#include <stddef.h>

void itoa(int n, char * str);

size_t puts(const char * str);
size_t putc(char c);
size_t puti(int num, int base, bool upper);
size_t putu(unsigned int num, unsigned int base, bool upper);

size_t printf(const char * fmt, ...);

#endif // STDIO_H
