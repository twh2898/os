#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

enum TIMER_FREQ {
    TIMER_FREQ_S  = 1,
    TIMER_FREQ_MS = 1000,
};

void init_timer(uint32_t freq);

void start_timer(uint32_t ticks);
void start_timer_ns(uint32_t ns);
void start_timer_ms(uint32_t ms);

uint32_t get_ticks();

uint32_t get_time_s();
uint32_t get_time_ms();
uint32_t get_time_ns();

#endif // TIMER_H
