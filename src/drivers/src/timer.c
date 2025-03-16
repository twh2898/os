#include "drivers/timer.h"

#include "cpu/isr.h"
#include "cpu/ports.h"
#include "ebus.h"
#include "libc/signal.h"
#include "libc/stdio.h"

// https://wiki.osdev.org/PIT

#define PIT_CTR0_PORT 0x40
#define PIT_CTR1_PORT 0x41
#define PIT_CTR2_PORT 0x42
#define PIT_CTL_PORT  0x43

#define BASE_FREQ 1193180

uint32_t __tick      = 0;
uint32_t __countdown = 0;
uint32_t __freq      = 0;

static void timer_callback(registers_t * regs) {
    __tick++;
    if (__countdown) {
        __countdown--;
        if (!__countdown) {
            ebus_event_t e;
            e.event_id   = EBUS_EVENT_TIMER;
            e.timer.time = __tick;
            queue_event(&e);
        }
    }
}

void init_timer(uint32_t freq) {
    __tick      = 0;
    __countdown = 0;
    __freq      = freq;

    /* Install the function we just wrote */
    register_interrupt_handler(IRQ0, timer_callback);

    /* Get the PIT value: hardware clock at 1193180 Hz */
    uint32_t divisor = BASE_FREQ / freq;
    uint8_t  low     = (uint8_t)(divisor & 0xFF);
    uint8_t  high    = (uint8_t)((divisor >> 8) & 0xFF);
    /* Send the command */
    port_byte_out(PIT_CTL_PORT, 0x36); /* Command port */
    port_byte_out(PIT_CTR0_PORT, low);
    port_byte_out(PIT_CTR0_PORT, high);
}

void start_timer(uint32_t ticks) {
    __countdown = ticks;
}
void start_timer_ns(uint32_t ns) {
    __countdown = ns * __freq / 1000000000;
}

void start_timer_ms(uint32_t ms) {
    __countdown = ms * __freq / 1000;
}

uint32_t get_ticks() {
    return __tick;
}

uint32_t get_time_s() {
    return __tick / __freq;
}

uint32_t get_time_ms() {
    return (__tick * 1000) / __freq;
}

uint32_t get_time_ns() {
    return (__tick * 1000000000) / __freq;
}
