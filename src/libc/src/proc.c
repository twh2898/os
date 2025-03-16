#include "libc/proc.h"

#include "libk/sys_call.h"

void proc_exit(uint8_t code) {
    _sys_proc_exit(code);
}

void proc_abort(uint8_t code, const char * msg) {
    _sys_proc_abort(code, msg);
}

NO_RETURN void proc_panic(const char * msg, const char * file, unsigned int line) {
    _sys_proc_panic(msg, file, line);
}

int getpid(void) {
    return _sys_proc_getpid();
}
