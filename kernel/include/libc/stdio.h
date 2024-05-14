#ifndef STDIO_H
#define STDIO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

size_t itoa(int32_t n, char * str);
// size_t ltoa(int64_t n, char * str);

size_t puts(const char * str);
size_t putc(char c);
size_t puti(int32_t num, uint8_t base, bool upper);
// size_t putli(int64_t num, uint8_t base, bool upper);
size_t putu(uint32_t num, uint8_t base, bool upper);
// size_t putlu(uint64_t num, uint8_t base, bool upper);

size_t printf(const char * fmt, ...);

#endif // STDIO_H
