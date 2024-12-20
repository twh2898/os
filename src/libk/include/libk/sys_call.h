#ifndef FOO_H
#define FOO_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#ifdef TESTING
#define NO_RETURN
#else
#define NO_RETURN _Noreturn
#endif

void * _malloc(size_t size);
void * _realloc(void * ptr, size_t size);
void   _free(void * ptr);

NO_RETURN void _proc_exit(uint8_t code);
NO_RETURN void _proc_abort(uint8_t code, const char * msg);
NO_RETURN void _proc_panic(const char * msg, const char * file, unsigned int line);

size_t _putc(char c);
size_t _puts(const char * str);

#endif // FOO_H
