#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stddef.h>

int    kmemcmp(const void * lhs, const void * rhs, size_t n);
void * kmemcpy(void * dest, const void * src, size_t n);
void * kmemmove(void * dest, const void * src, size_t n);
void * kmemset(void * dest, int value, size_t n);

size_t kstrlen(const char * str);
size_t knstrlen(const char * str, int max);
int    kstrcmp(const char * lhs, const char * rhs);
char * kstrfind(const char * str, int c);
char * kstrtok(char * str, const char * delim);

int katoi(const char * str);

#endif // STRING_H
