#include "cpu/timer.h"

#include "cpu/isr.h"
#include "cpu/ports.h"
#include "libc/stdio.h"

// https://wiki.osdev.org/PIT

#define PIT_CTR0_PORT 0x40
#define PIT_CTR1_PORT 0x41
#define PIT_CTR2_PORT 0x42
#define PIT_CTL_PORT  0x43

uint32_t tick = 0;

static void timer_callback(registers_t regs) {
    tick++;
}

void init_timer(uint32_t freq) {
    /* Install the function we just wrote */
    register_interrupt_handler(IRQ0, timer_callback);

    /* Get the PIT value: hardware clock at 1193180 Hz */
    uint32_t divisor = 1193180 / freq;
    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);
    /* Send the command */
    port_byte_out(PIT_CTL_PORT, 0x36); /* Command port */
    port_byte_out(PIT_CTR0_PORT, low);
    port_byte_out(PIT_CTR0_PORT, high);
}

uint32_t get_ticks() {
    return tick;
}
