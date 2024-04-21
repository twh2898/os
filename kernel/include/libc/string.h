#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

int memcmp(const void * lhs, const void * rhs, size_t n);
void * memcpy(void * dest, const void * src, size_t n);
void * memmove(void * dest, const void * src, size_t n);
void * memset(void * dest, uint8_t value, size_t n);
size_t strlen(const char * str);

#endif // STRING_H
