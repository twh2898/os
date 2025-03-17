#include "drivers/timer.h"

#include "cpu/isr.h"
#include "cpu/ports.h"
#include "ebus.h"
#include "libc/datastruct/array.h"
#include "libc/signal.h"
#include "libc/stdio.h"

// https://wiki.osdev.org/PIT

#define PIT_CTR0_PORT 0x40
#define PIT_CTR1_PORT 0x41
#define PIT_CTR2_PORT 0x42
#define PIT_CTL_PORT  0x43

#define BASE_FREQ 1193180

typedef struct _timer {
    int      id;
    uint32_t count;
} timer_t;

uint32_t __tick    = 0;
uint32_t __freq    = 0;
int      __next_id = 1;

arr_t timers; // timer_t

static void timer_callback(registers_t * regs) {
    __tick++;
    for (int i = 0; i < arr_size(&timers); i++) {
        timer_t * timer = arr_at(&timers, i);
        timer->count--;
        if (timer->count == 0) {
            ebus_event_t e;
            e.event_id   = EBUS_EVENT_TIMER;
            e.timer.id   = timer->id;
            e.timer.time = __tick;
            queue_event(&e);
            arr_remove(&timers, i, 0);
            i--; // Account for the reduced size after insert
        }
    }
}

void init_timer(uint32_t freq) {
    __tick    = 0;
    __freq    = freq;
    __next_id = 1;

    if (arr_create(&timers, 4, sizeof(timer_t))) {
        return;
    }

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

int start_timer(uint32_t ticks) {
    timer_t t;
    t.id    = __next_id++;
    t.count = ticks;
    if (arr_insert(&timers, arr_size(&timers), &t)) {
        return -1;
    }
    return t.id;
}

int start_timer_ns(uint32_t ns) {
    return start_timer(ns * __freq / 1000000000);
}

int start_timer_ms(uint32_t ms) {
    return start_timer(ms * __freq / 1000);
}

void stop_timer(int id) {
    for (int i = 0; i < arr_size(&timers); i++) {
        timer_t * t = arr_at(&timers, i);
        if (t->id == id) {
            arr_remove(&timers, i, 0);
            return;
        }
    }
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
