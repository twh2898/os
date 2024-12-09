#ifndef FOO_H
#define FOO_H

#include <stdarg.h>
#include <stdint.h>

void sys_call_1();

uint32_t sys_call_puts(char * str);
uint32_t sys_call_printf(char * fmt, ...);
void sys_call_num(uint32_t n);

#endif // FOO_H
