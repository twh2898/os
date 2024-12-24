#include "libk/sys_call.h"

#include <stdint.h>

#include "libk/defs.h"

#define PTR2UINT(PTR)   ((uint32_t)(PTR))
#define UINT2PTR(UINT)  ((void *)(UINT))
#define LUINT2PTR(UINT) UINT2PTR((uint32_t)(UINT))

extern uint32_t       send_interrupt(uint32_t int_no, ...);
extern NO_RETURN void send_interrupt_noret(uint32_t int_no, ...);

void * _malloc(size_t size) {
    return UINT2PTR(send_interrupt(SYS_INT_MEM_MALLOC, size));
}

void * _realloc(void * ptr, size_t size) {
    return UINT2PTR(send_interrupt(SYS_INT_MEM_REALLOC, ptr, size));
}

void _free(void * ptr) {
    send_interrupt(SYS_INT_MEM_FREE, ptr);
}

void _proc_exit(uint8_t code) {
    _puts("Proc exit\n");
    send_interrupt_noret(SYS_INT_PROC_EXIT, code);
}

void _proc_abort(uint8_t code, const char * msg) {
    _puts("Proc abort\n");
    send_interrupt_noret(SYS_INT_PROC_ABORT, code, msg);
}

void _proc_panic(const char * msg, const char * file, unsigned int line) {
    _puts("Proc panic\n");
    send_interrupt_noret(SYS_INT_PROC_PANIC, msg, file, line);
}

void _register_signals(void * callback) {
    send_interrupt(SYS_INT_PROC_REG_SIG, callback);
}

size_t _putc(char c) {
    return send_interrupt(SYS_INT_STDIO_PUTC, c);
}

size_t _puts(const char * str) {
    return send_interrupt(SYS_INT_STDIO_PUTS, str);
}
