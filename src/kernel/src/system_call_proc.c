#include "kernel/system_call_proc.h"

#include <stddef.h>

#include "defs.h"
#include "drivers/vga.h"
#include "ebus.h"
#include "kernel.h"
#include "libc/stdio.h"
#include "libk/defs.h"
#include "process.h"

int sys_call_proc_cb(uint16_t int_no, void * args_data, registers_t * regs) {
    int res = 0;

    switch (int_no) {
        case SYS_INT_PROC_EXIT: {
            struct _args {
                uint8_t code;
            } args = *(struct _args *)args_data;
            printf("Proc exit with code %u\n", args.code);
            kernel_exit();
        } break;

        case SYS_INT_PROC_ABORT: {
            struct _args {
                uint8_t      code;
                const char * msg;
            } args = *(struct _args *)args_data;
            printf("Proc abort with code %u\n", args.code);
            puts(args.msg);
            kernel_exit();
        } break;

        case SYS_INT_PROC_PANIC: {
            struct _args {
                const char * msg;
                const char * file;
                unsigned int line;
            } args = *(struct _args *)args_data;
            vga_color(VGA_FG_WHITE | VGA_BG_RED);
            vga_puts("[PANIC]");
            if (args.file) {
                vga_putc('[');
                vga_puts(args.file);
                vga_puts("]:");
                vga_putu(args.line);
            }
            if (args.msg) {
                vga_putc(' ');
                vga_puts(args.msg);
            }
            vga_cursor_hide();
            asm("cli");
            for (;;) {
                asm("hlt");
            }
        } break;

        case SYS_INT_PROC_REG_SIG: {
            struct _args {
                signals_master_cb_t cb;
            } args = *(struct _args *)args_data;
            tmp_register_signals_cb(args.cb);
        } break;

        case SYS_INT_PROC_GETPID: {
            process_t * p = get_current_process();
            if (p) {
                res = p->pid;
            }
            else {
                res = -1;
            }
        } break;

        case SYS_INT_PROC_QUEUE_EVENT: {
            struct _args {
                ebus_event_t * event;
            } args = *(struct _args *)args_data;

            ebus_push(get_kernel_ebus(), args.event);
        } break;

        case SYS_INT_PROC_YIELD: {
            struct _args {
                int filter;
            } args = *(struct _args *)args_data;

            process_t * proc = get_current_process();
            process_yield(proc, args.filter);
        };
    }

    return res;
}
