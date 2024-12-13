#include "libc/stdio.h"

void foo() {
    uint32_t res = puts("Hello\n");

    res = printf("Hello 0x%X\n", res);

    printf("There are %u res and it has %c with %d left\n", res, 'c', 1800);
}

void __start() {
    foo();
}
