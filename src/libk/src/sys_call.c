#include "libk/sys_call.h"

#include <stdint.h>

#include "libk/defs.h"

#define PTR2UINT(PTR)   ((uint32_t)(PTR))
#define UINT2PTR(UINT)  ((void *)(UINT))
#define LUINT2PTR(UINT) UINT2PTR((uint32_t)(UINT))

extern uint32_t       send_interrupt(uint32_t int_no, ...);
extern NO_RETURN void send_interrupt_noret(uint32_t int_no, ...);

int _sys_io_open(const char * path, const char * mode) {
    return send_interrupt(SYS_INT_IO_OPEN, path, mode);
}

int _sys_io_close(int handle) {
    return send_interrupt(SYS_INT_IO_CLOSE, handle);
}

int _sys_io_read(int handle, char * buff, size_t count) {
    return send_interrupt(SYS_INT_IO_READ, handle, buff, count);
}

int _sys_io_write(int handle, const char * buff, size_t count) {
    return send_interrupt(SYS_INT_IO_WRITE, handle, buff, count);
}

int _sys_io_seek(int handle, int pos, int seek) {
    return send_interrupt(SYS_INT_IO_SEEK, handle, pos, seek);
}

int _sys_io_tell(int handle) {
    return send_interrupt(SYS_INT_IO_TELL, handle);
}

void * _page_alloc(size_t count) {
    return UINT2PTR(send_interrupt(SYS_INT_MEM_PAGE_ALLOC, count));
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
