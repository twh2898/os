#ifndef STDIO_H
#define STDIO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

size_t itoa(int32_t n, char * str);
size_t ltoa(int64_t n, char * str);

size_t kputs(const char * str);
size_t kputc(char c);
size_t kputi(int32_t num, uint8_t base, bool upper);
size_t kputli(int64_t num, uint8_t base, bool upper);
size_t kputu(uint32_t num, uint8_t base, bool upper);
size_t kputlu(uint64_t num, uint8_t base, bool upper);

size_t kprintf(const char * fmt, ...);

size_t kprint_hexblock(const uint8_t * data, size_t count, size_t addr_offset);

#endif // STDIO_H
