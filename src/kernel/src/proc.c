#include "proc.h"

#include "libc/memory.h"
#include "libc/string.h"

process_t * proc_new(uint32_t pid, const char * command, uint16_t flags) {
    process_t * proc = kmalloc(sizeof(process_t));
    if (proc) {
        proc->pid            = pid;
        proc->flags          = PROCESS_FLAGS_ACTIVE;
        proc->page_dir_paddr = 0;
        kmemset(&proc->tss, 0, sizeof(tss_entry_t));

        size_t len = kstrlen(command);
        if (len > 256)
            len = 256;

        kmemcpy(proc->command, command, len);
        proc->command[len] = 0;
    }
    return proc;
}

void proc_free(process_t * proc) {
    kfree(proc);
}
