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
            } * args = (struct _args *)args_data;
            printf("Proc exit with code %u\n", args->code);
            process_t * proc = get_current_process();
            enable_interrupts();
            if (kernel_close_process(proc)) {
                KPANIC("Kernel could not close process");
            }
            kernel_next_task();
            KPANIC("Unexpected return from kernel_close_process");
        } break;

        case SYS_INT_PROC_ABORT: {
            struct _args {
                uint8_t      code;
                const char * msg;
            } * args = (struct _args *)args_data;
            printf("Proc abort with code %u\n", args->code);
            puts(args->msg);
            process_t * proc = get_current_process();
            enable_interrupts();
            if (kernel_close_process(proc)) {
                KPANIC("Kernel could not close process");
            }
            kernel_next_task();
            KPANIC("Unexpected return from kernel_close_process");
        } break;

        case SYS_INT_PROC_PANIC: {
            struct _args {
                const char * msg;
                const char * file;
                unsigned int line;
            } * args = (struct _args *)args_data;
            vga_color(VGA_FG_WHITE | VGA_BG_RED);
            vga_puts("[PANIC]");
            if (args->file) {
                vga_putc('[');
                vga_puts(args->file);
                vga_puts("]:");
                vga_putu(args->line);
            }
            if (args->msg) {
                vga_putc(' ');
                vga_puts(args->msg);
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
            } * args = (struct _args *)args_data;
            tmp_register_signals_cb(args->cb);
        } break;

        case SYS_INT_PROC_GETPID: {
            process_t * p = get_current_process();
            if (!p) {
                KPANIC("Failed to find current process");
            }
            res = p->pid;
        } break;

        case SYS_INT_PROC_QUEUE_EVENT: {
            struct _args {
                ebus_event_t * event;
            } * args = (struct _args *)args_data;

            ebus_push(get_kernel_ebus(), args->event);
        } break;

        case SYS_INT_PROC_YIELD: {
            struct _args {
                int filter;
            } * args = (struct _args *)args_data;

            // TODO clear iret from stack?
            process_t * proc = get_current_process();
            proc->entrypoint = regs->eip;
            enable_interrupts();
            process_yield(proc, regs->esp, args->filter);
        };
    }

    return res;
}
