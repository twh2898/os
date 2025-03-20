#ifndef KERNEL_PROCESS_MANAGER_H
#define KERNEL_PROCESS_MANAGER_H

#include <stddef.h>
#include <stdint.h>

#include "libc/datastruct/array.h"
#include "process.h"

typedef struct _proc_man {
    arr_t task_list; // process_t *
    /// Waiting for an event
    arr_t       waiting; // process_t *
    process_t * active;
} proc_man_t;

int pm_create(proc_man_t * pm);

process_t * pm_get_pid(proc_man_t * pm, int pid);

int pm_add_proc(proc_man_t * pm, process_t * proc);
int pm_remove_proc(proc_man_t * pm, int pid);

int pm_switch_process(proc_man_t * pm);
int pm_resume_process(proc_man_t * pm);

process_t * get_current_process();

int kernel_add_task(process_t * proc);
int kernel_next_task();
int kernel_close_process(process_t * proc);
int kernel_set_current_task(process_t * proc);

typedef int (*_proc_call_t)(void * data);

int kernel_call_as_proc(int pid, _proc_call_t fn, void * data);

int kernel_switch_task(int next_pid);

#endif // KERNEL_PROCESS_MANAGER_H
