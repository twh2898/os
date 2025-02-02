#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "cpu/isr.h"
#include "defs.h"

typedef uint32_t (*sys_interrupt_handler_t)(uint16_t interrupt_no, void * args_data, registers_t * regs);

void init_system_interrupts(uint8_t isr_interrupt_no);

void system_interrupt_register(uint8_t family, sys_interrupt_handler_t handler);

#endif // INTERRUPTS_H
