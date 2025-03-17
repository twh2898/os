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

void queue_event(ebus_event_t * event) {
    event->source_pid = getpid();
    _sys_queue_event(event);
}

int yield(int filter) {
    return _sys_yield(filter);
}

int getpid(void) {
    return _sys_proc_getpid();
}
