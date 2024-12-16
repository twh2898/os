#ifndef FOO_H
#define FOO_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

void * _malloc(size_t size);
void * _realloc(void * ptr, size_t size);
void   _free(void * ptr);

// void _exit(uint8_t code);

size_t _putc(char c);
size_t _puts(const char * str);

#endif // FOO_H
