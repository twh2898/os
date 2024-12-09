#include "libc/sys_call.h"

void foo() {
    sys_call_1();
    sys_call_num(88);
    uint32_t res = sys_call_puts("Hello\n");
    sys_call_num(res);
    res = sys_call_printf("Hello %d\n", 42);
    sys_call_num(res);
    sys_call_printf("There are %u res and it has %c with %d left\n", res, 'c', 1800);
}

void __start() {
    foo();
}
