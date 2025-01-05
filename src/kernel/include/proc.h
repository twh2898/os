#ifndef PROC_H
#define PROC_H

#include <stdint.h>

#include "cpu/isr.h"
#include "cpu/mmu.h"

typedef void (*signals_master_callback)(int);

typedef struct _process {
    uint32_t pid;
    uint32_t next_heap_page;
    uint32_t stack_page_count;

    uint32_t esp;
    uint32_t cr3;
    uint32_t ss0;

    registers_t regs;

    signals_master_callback sys_call_callback;

    struct _process * next_proc;
} process_t;

int process_create(process_t * proc);
int process_free(process_t * proc);

/**
 * @brief Add `count` pages to the process heap.
 *
 * @param proc pointer to the process object
 * @param count number of pages to add
 * @return pointer to the new pages in virtual memory
 */
void * process_add_pages(process_t * proc, size_t count);

/**
 * @brief Add a single page to expand the process stack
 *
 * @param proc pointer to the process object
 * @return int 0 for success
 */
int process_grow_stack(process_t * proc);

// Old

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
