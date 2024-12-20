#include "libc/process.h"

#include "libk/sys_call.h"

void proc_exit(uint8_t code) {
    _proc_exit(code);
}

void proc_abort(uint8_t code, const char * msg) {
    _proc_abort(code, msg);
}

NO_RETURN void proc_panic(const char * msg, const char * file, unsigned int line) {
    _proc_panic(msg, file, line);
}
