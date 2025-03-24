#include "exec.h"

#include "cpu/mmu.h"
#include "cpu/tss.h"
#include "kernel.h"
#include "libc/memory.h"
#include "libc/proc.h"
#include "libc/stdio.h"
#include "libc/string.h"
#include "paging.h"
#include "process.h"
#include "ram.h"

typedef int (*ff_t)(size_t argc, char ** argv);

extern _Noreturn void jump_proc(uint32_t cr3, uint32_t esp, uint32_t call);

int command_exec(uint8_t * buff, size_t size, size_t argc, char ** argv) {
    process_t * proc = kmalloc(sizeof(process_t));

    if (process_create(proc)) {
        puts("Failed to create process\n");
        return -1;
    }

    if (process_load_heap(proc, buff, size)) {
        puts("Failed to load\n");
        process_free(proc);
        return -1;
    }

    process_set_entrypoint(proc, UINT2PTR(VADDR_USER_MEM));
    process_add_pages(proc, 32);
    pm_add_proc(kernel_get_proc_man(), proc);

    int res = pm_resume_process(kernel_get_proc_man(), proc->pid, 0);

    pm_remove_proc(kernel_get_proc_man(), proc->pid);
    process_free(proc);

    return res;
}
