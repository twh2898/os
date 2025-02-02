#include "kernel/system_call_mem.h"

#include <stddef.h>

#include "kernel.h"
#include "libk/defs.h"
#include "process.h"

int sys_call_mem_cb(uint16_t int_no, void * args_data, registers_t * regs) {
    int res = 0;

    switch (int_no) {
        case SYS_INT_MEM_PAGE_ALLOC: {
            struct _args {
                size_t count;
            } args = *(struct _args *)args_data;

            process_t * curr_proc = get_current_process();

            res = PTR2UINT(process_add_pages(curr_proc, args.count));
        } break;
    }

    return res;
}
