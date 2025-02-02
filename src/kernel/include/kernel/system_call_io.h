#ifndef KERNEL_SYSTEM_CALL_IO_H
#define KERNEL_SYSTEM_CALL_IO_H

#include "kernel/system_call.h"

uint32_t sys_call_io_cb(uint16_t int_no, void * args_data, registers_t * regs);

#endif // KERNEL_SYSTEM_CALL_IO_H
