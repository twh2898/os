#include "libc/proc.h"

#include "libk/sys_call.h"

void proc_exit(uint8_t code) {
    _proc_exit(code);
}
