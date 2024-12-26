#ifndef PROC_H
#define PROC_H

#include <stdint.h>

#include "cpu/mmu.h"
#include "libc/datastruct/array.h"

enum PROCESS_FLAGS {
    PROCESS_FLAGS_ACTIVE = 0x1,
    PROCESS_FLAGS_ERROR  = 0x2,
    PROCESS_FLAGS_DEAD   = 0x4,
};

typedef struct _process {
    uint32_t eip;
    uint32_t esp;
    uint32_t cr3;
    uint32_t ss0;
    uint16_t pid;
    arr_t *  pages; // array<void *> pages owned by this procs page dir (not including special or kernel)

    struct _process * next_proc;
} process_t;

process_t * proc_new(void * fn, uint32_t ss0);
process_t * proc_from_ptr(uint32_t eip, uint32_t esp, uint32_t cr3, uint32_t ss0);
void        proc_free(process_t * proc);

extern void set_first_task(process_t * next_proc);
extern void switch_to_task(process_t * next_proc);

typedef struct _proc_man {
    process_t * idle_task;
    process_t * task_begin;
    process_t * curr_task;
} proc_man_t;

proc_man_t * proc_man_new();

void proc_man_set_idle(proc_man_t * pm, process_t * proc);
void proc_man_set_new_idle(proc_man_t * pm, uint32_t eip, uint32_t esp, uint32_t cr3, uint32_t ss0);
int  proc_man_task_from_ptr(void * fn);
void proc_man_switch_to_idle(proc_man_t * pm);

#endif // PROC_H
