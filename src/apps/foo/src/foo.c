#include "libc/sys_call.h"

void foo() {
    sys_call_num(88);
    uint32_t res = sys_call_print("Hello\n");
    sys_call_num(res);
}

void __start() {
    foo();
}
