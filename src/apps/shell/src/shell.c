#include "libc/memory.h"
#include "libc/proc.h"
#include "libc/signal.h"
#include "libc/stdio.h"

void foo_callback();

static int i;

int __start(size_t argc, char ** argv) {
    i = 0;
    register_signal(PROC_SIGNALS_FOO, foo_callback);

    puts("Welcome to shell!\n$ ");

    for (;;) {
        ebus_event_t event;
        int          ev = pull_event(EBUS_EVENT_KEY, &event);
        if (ev == EBUS_EVENT_KEY) {
            puts("Key ");
        }
        // if (i) {
        //     i--;
        //     puts("YAY!\n");
        // }
        // asm("hlt");
    }

    return 0;
}

void foo_callback() {
    puts("Got a shell callback!\n");
    i++;
}
