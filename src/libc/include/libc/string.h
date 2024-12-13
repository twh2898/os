#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

#ifdef TESTING
#define OS_FN(func) OS_##func
#else
#define OS_FN(func) func
#endif

int    OS_FN(memcmp)(const void * lhs, const void * rhs, size_t n);
void * OS_FN(memcpy)(void * dest, const void * src, size_t n);
void * OS_FN(memmove)(void * dest, const void * src, size_t n);
void * OS_FN(memset)(void * dest, uint8_t value, size_t n);

int    OS_FN(strlen)(const char * str);
int    OS_FN(nstrlen)(const char * str, int max);
int    OS_FN(strcmp)(const char * lhs, const char * rhs);
int    OS_FN(strfind)(const char * str, size_t start, char c);
char * OS_FN(strtok)(char * str, char * delim);

int OS_FN(atoi)(const char * str);
int OS_FN(atoib)(const char * str, int base);

#endif // STRING_H
