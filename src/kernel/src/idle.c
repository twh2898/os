#include "idle.h"

#include "kernel.h"
#include "libc/memory.h"
#include "libc/proc.h"
#include "libc/stdio.h"

static void idle_loop();

process_t * init_idle() {
    process_t * proc = kmalloc(sizeof(process_t));
    if (!proc || process_create(proc)) {
        return 0;
    }

    process_set_entrypoint(proc, idle_loop);
    proc->state = PROCESS_STATE_LOADED;

    return proc;
}

static void idle_loop() {
    for (;;) {
        // printf("idle %u\n", getpid());
        ebus_cycle(get_kernel_ebus());
        asm("hlt");
        int curr_pid = get_current_process()->pid;
        yield();
    }
}
