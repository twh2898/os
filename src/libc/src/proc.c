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

int pull_event(int filter, ebus_event_t * event_out) {
    if (_sys_yield(filter, event_out)) {
        return -1;
    }
    if (event_out) {
        return event_out->event_id;
    }
    return 0;
}

void yield() {
    _sys_yield(0, 0);
}

int getpid(void) {
    return _sys_proc_getpid();
}
