#include "proc.h"

#include "libc/string.h"
#include "memory.h"

process_t * proc_new(uint32_t eip, uint32_t esp, uint32_t cr3, uint32_t ss0, uint32_t pid, const char * command, uint16_t flags) {
    process_t * proc = impl_kmalloc(sizeof(process_t));
    if (proc) {
        proc->eip = eip;
        proc->esp = esp;
        proc->cr3 = cr3;
        proc->ss0 = ss0;
        proc->pid = pid;
    }
    return proc;
}

void proc_free(process_t * proc) {
    impl_kfree(proc);
}
