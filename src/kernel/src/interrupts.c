#include "interrupts.h"

#include "cpu/isr.h"
#include "drivers/vga.h"
#include "libc/memory.h"
#include "libc/process.h"
#include "libc/stdio.h"
#include "libc/string.h"
#include "libk/defs.h"

#define MAX_CALLBACKS 0x100
sys_interrupt_handler_t callbacks[MAX_CALLBACKS];

static void callback(registers_t regs);

void init_system_interrupts(uint8_t isr_interrupt_no) {
    kmemset(callbacks, 0, sizeof(callbacks));
    register_interrupt_handler(isr_interrupt_no, callback);
}

void system_interrupt_register(uint8_t family, sys_interrupt_handler_t handler) {
    if (family > MAX_CALLBACKS) {
        PANIC("Out of range interrupt family");
    }
    callbacks[family] = handler;
}

static void callback(registers_t regs) {
    uint32_t res = 0;

    uint16_t int_no = regs.eax & 0xffff;
    uint8_t  family = (regs.eax >> 8) & 0xff;

    sys_interrupt_handler_t handler = callbacks[family];

    if (handler) {
        res = handler(int_no, &regs);
    }
    else {
        vga_print("Unknown interrupt: 0x");
        vga_putx(int_no);
        // print_trace(&regs);
        PANIC("UNKNOWN INTERRUPT");
    }

    // Get access to stack push of eax
    uint32_t * ret = UINT2PTR(regs.esp - 4);
    *ret           = res;
}
