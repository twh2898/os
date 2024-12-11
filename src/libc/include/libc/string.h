#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

int memcmp(const void * lhs, const void * rhs, size_t n);
void * memcpy(void * dest, const void * src, size_t n);
void * memmove(void * dest, const void * src, size_t n);
void * memset(void * dest, uint8_t value, size_t n);

int strlen(const char * str);
int nstrlen(const char * str, int max);
int strcmp(const char * lhs, const char * rhs);
int strfind(const char * str, size_t start, char c);
char * strtok(char * str, char * delim);

int atoi(const char * str);
int atoib(const char * str, int base);

#endif // STRING_H
