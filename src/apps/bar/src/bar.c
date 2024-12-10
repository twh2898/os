#include <stddef.h>
#include <stdint.h>

#include "libc/stdio.h"

int __start(size_t argc, char ** argv) {
    kprintf("Bar got %u arguments\n", argc);

    for (size_t i = 0; i < argc; i++) {
        kprintf("Argument %u is ", i);
        kputs(argv[i]);
        kputc('\n');
    }

    return 0;
}
