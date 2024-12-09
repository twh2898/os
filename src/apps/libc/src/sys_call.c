#include "libc/sys_call.h"

#include <stdint.h>

#define SYS_INT_IO_OPEN  0x0100
#define SYS_INT_IO_CLOSE 0x0101
#define SYS_INT_IO_READ  0x0102
#define SYS_INT_IO_WRITE 0x0103

#define SYS_INT_MEM_MALLOC  0x0200
#define SYS_INT_MEM_CALLOC  0x0201
#define SYS_INT_MEM_REALLOC 0x0202
#define SYS_INT_MEM_FREE    0x0203

extern int send_interrupt(uint16_t int_no, ...);

void sys_call_1() {
    send_interrupt(14);
}

uint32_t sys_call_print(char * str) {
    uint32_t res = send_interrupt(15, str);
    return res;
}

void sys_call_num(uint32_t n) {
    send_interrupt(16, n);
}
