#include "exec.h"

#include "cpu/mmu.h"
#include "cpu/tss.h"
#include "libc/memory.h"
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

    puts("Go for call\n");

    ff_t call = UINT2PTR(VADDR_USER_MEM);

    // mmu_change_dir(proc->cr3);

    tss_get_entry(0)->esp0 = proc->esp0;
    tss_get_entry(0)->cr3  = proc->cr3;
    tss_get_entry(0)->esp  = proc->esp;

    jump_proc(proc->cr3, proc->esp, VADDR_USER_MEM);

    int res = call(argc, argv);

    puts("Done\n");

    process_free(proc);

    puts("All good!\n");

    return res;
}
