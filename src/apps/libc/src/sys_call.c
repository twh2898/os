#include "libc/sys_call.h"

#include "sysint.h"

void sys_call_1() {
    send_interrupt(14);
}
