#ifndef FOO_H
#define FOO_H

#include <stdint.h>

void sys_call_1();

uint32_t sys_call_print(char * str);
void sys_call_num(uint32_t n);

#endif // FOO_H
