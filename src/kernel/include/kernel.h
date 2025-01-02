#ifndef KERNEL_H
#define KERNEL_H

#include <stddef.h>
#include <stdint.h>

#include "proc.h"

typedef struct _kernel {
    uint32_t   ram_table;
    size_t     ram_table_count;
    uint32_t   cr3;
    process_t  proc;
    proc_man_t pm;
} kernel_t;

int kernel_init(kernel_t * kernel);

#endif // KERNEL_H
