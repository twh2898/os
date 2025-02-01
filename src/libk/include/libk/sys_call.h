#ifndef LIBK_SYS_CALL_H
#define LIBK_SYS_CALL_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#ifdef TESTING
#define NO_RETURN
#else
#define NO_RETURN _Noreturn
#endif

int _sys_io_open(const char * path, const char * mode);
int _sys_io_close(int handle);
int _sys_io_read(int handle, char * buff, size_t count);
int _sys_io_write(int handle, const char * buff, size_t count);
int _sys_io_seek(int handle, int pos, int seek);
int _sys_io_tell(int handle);

void * _page_alloc(size_t count);

NO_RETURN void _proc_exit(uint8_t code);
NO_RETURN void _proc_abort(uint8_t code, const char * msg);
NO_RETURN void _proc_panic(const char * msg, const char * file, unsigned int line);

void _register_signals(void * callback);

size_t _putc(char c);
size_t _puts(const char * str);

#endif // LIBK_SYS_CALL_H
