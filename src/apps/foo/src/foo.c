#include "libc/stdio.h"

void foo() {
    uint32_t res = kputs("Hello\n");
    res = kprintf("Hello 0x%X\n", res);
    kprintf("There are %u res and it has %c with %d left\n", res, 'c', 1800);
}

void __start() {
    foo();
}
