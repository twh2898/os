#include "libc/sys_call.h"

#include <stdint.h>

extern void apps_send_interrupt(uint16_t int_no, ...);

void sys_call_1() {
    apps_send_interrupt(14);
}

void sys_call_print(char * str) {
    apps_send_interrupt(15, str);
}
