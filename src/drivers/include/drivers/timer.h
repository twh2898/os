#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

enum TIMER_FREQ {
    TIMER_FREQ_S  = 1,
    TIMER_FREQ_MS = 1000,
};

void init_timer(uint32_t freq);

/**
 * @brief
 *
 * @param ticks
 * @return int id, < 0 for fail
 */
int start_timer(uint32_t ticks);
int start_timer_ns(uint32_t ns);
int start_timer_ms(uint32_t ms);

void stop_timer(int id);

uint32_t get_ticks();

uint32_t get_time_s();
uint32_t get_time_ms();
uint32_t get_time_ns();

#endif // TIMER_H
