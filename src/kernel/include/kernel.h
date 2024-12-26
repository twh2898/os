#ifndef KERNEL_H
#define KERNEL_H

#include <stddef.h>
#include <stdint.h>

#include "proc.h"

typedef struct _kernel {
    void *     ram_table;
    proc_man_t pm;
} kernel_t;

int kernel_init(kernel_t * kernel);

#endif // KERNEL_H
