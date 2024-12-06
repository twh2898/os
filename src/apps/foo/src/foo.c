#include "libc/sys_call.h"

void foo() {
    sys_call_print("Hello World!");
}

void __start() {
    foo();
}
