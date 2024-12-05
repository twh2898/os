#include "sysint.h"
#include "libc/sys_call.h"

void sys_call_1() {
    send_interrupt(14);
}

void foo() {
    sys_call_1(14);
}

void __start() {
    foo();
}
