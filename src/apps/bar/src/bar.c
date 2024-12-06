#include "libc/sys_call.h"

void bar() {
    sys_call_1();
}

void __start() {
    bar();
}
