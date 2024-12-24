#include "libc/memory.h"
#include "libc/stdio.h"

int __start(size_t argc, char ** argv) {
    puts("Welcome to shell!\n$ ");

    for (;;) {
        asm("hlt");
    }

    return 0;
}
