#include "proc.h"

#include "libc/string.h"
#include "memory.h"

process_t * proc_new(uint32_t pid, const char * command, uint16_t flags) {
    process_t * proc = impl_kmalloc(sizeof(process_t));
    if (proc) {
        proc->esp  = 0;
        proc->cr3  = 0;
        proc->esp0 = 0;
        proc->pid  = pid;
    }
    return proc;
}

void proc_free(process_t * proc) {
    impl_kfree(proc);
}
