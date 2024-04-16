#ifndef STDIO_H
#define STDIO_H

#include <stdbool.h>

void itoa(int n, char * str);

int putc(char c);
int puti(int num, int base, bool upper);
int putu(unsigned int num, unsigned int base, bool upper);

int printf(const char * fmt, ...);

#endif // STDIO_H
