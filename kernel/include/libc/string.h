#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

int kmemcmp(const void * lhs, const void * rhs, size_t n);
void * kmemcpy(void * dest, const void * src, size_t n);
void * kmemmove(void * dest, const void * src, size_t n);
void * kmemset(void * dest, uint8_t value, size_t n);

int kstrlen(const char * str);
int knstrlen(const char * str, int max);
int kstrfind(const char * str, size_t start, char c);
char * kstrtok(char * str, char * delim);

int atoi(const char * str);
int atoib(const char * str, int base);

#endif // STRING_H
