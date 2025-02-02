#include "kernel/system_call_proc.h"

#include <stddef.h>

#include "defs.h"
#include "drivers/vga.h"
#include "kernel.h"
#include "libc/stdio.h"
#include "libk/defs.h"
#include "process.h"

uint32_t sys_call_proc_cb(uint16_t int_no, void * args_data, registers_t * regs) {
    uint32_t res = 0;

    switch (int_no) {
        case SYS_INT_PROC_EXIT: {
            struct _args {
                uint8_t code;
            } args = *(struct _args *)args_data;
            printf("Proc exit with code %u\n", args.code);
            // regs->eip = PTR2UINT(term_run);
            // kernel_exit();
        } break;

        case SYS_INT_PROC_ABORT: {
            struct _args {
                uint8_t      code;
                const char * msg;
            } args = *(struct _args *)args_data;
            printf("Proc exit with code %u\n", args.code);
            puts(args.msg);
            // regs->eip = PTR2UINT(term_run);
            // kernel_exit();
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
    }

    return 0;
}
