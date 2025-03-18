#ifndef KERNEL_PROCESS_MANAGER_H
#define KERNEL_PROCESS_MANAGER_H

#include <stddef.h>
#include <stdint.h>

#include "process.h"

typedef struct _proc_man {
    process_t * idle_task;
    process_t * task_begin;
    process_t * curr_task;
} proc_man_t;

int pm_create(proc_man_t * pm);

process_t * pm_get_pid(proc_man_t * pm, int pid);

int pm_add_proc(proc_man_t * pm, process_t * proc);
int pm_remove_proc(proc_man_t * pm, process_t * proc);

#endif // KERNEL_PROCESS_MANAGER_H
