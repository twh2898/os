#include "libk/sys_call.h"

#include <stdarg.h>
#include <stdint.h>

#define PTR2UINT(PTR)   ((uint32_t)(PTR))
#define UINT2PTR(UINT)  ((void *)(UINT))
#define LUINT2PTR(UINT) UINT2PTR((uint32_t)(UINT))

#define SYS_INT_IO_OPEN  0x0100
#define SYS_INT_IO_CLOSE 0x0101
#define SYS_INT_IO_READ  0x0102
#define SYS_INT_IO_WRITE 0x0103

#define SYS_INT_MEM_MALLOC  0x0200
#define SYS_INT_MEM_REALLOC 0x0201
#define SYS_INT_MEM_FREE    0x0202

#define SYS_INT_PROC_EXIT 0x0300

#define SYS_INT_STDIO_PUTC    0x1000
#define SYS_INT_STDIO_PUTS    0x1001
#define SYS_INT_STDIO_VPRINTF 0x1002

extern uint32_t send_interrupt(uint32_t int_no, ...);

void * _malloc(size_t size) {
    return UINT2PTR(send_interrupt(SYS_INT_MEM_MALLOC, size));
}

void * _realloc(void * ptr, size_t size) {
    return UINT2PTR(send_interrupt(SYS_INT_MEM_REALLOC, ptr, size));
}

void _free(void * ptr) {
    send_interrupt(SYS_INT_MEM_FREE, ptr);
}

void _exit(uint8_t code) {
    send_interrupt(SYS_INT_PROC_EXIT, code);
}

size_t _putc(char c) {
    return send_interrupt(SYS_INT_STDIO_PUTC, c);
}

size_t _puts(const char * str) {
    return send_interrupt(SYS_INT_STDIO_PUTS, str);
}

size_t _vprintf(const char * fmt, va_list params) {
    return send_interrupt(SYS_INT_STDIO_VPRINTF, fmt, params);
}
