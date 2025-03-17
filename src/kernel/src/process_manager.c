#include "process_manager.h"

int pm_create(proc_man_t * pm) {
    if (!pm) {
        return -1;
    }

    pm->idle_task  = 0;
    pm->task_begin = 0;
    pm->curr_task  = 0;

    return 0;
}

int pm_add_proc(proc_man_t * pm, process_t * proc) {
    if (!pm || !proc) {
        return -1;
    }

    process_t * curr = pm->curr_task;
    while (curr->next_proc && curr->next_proc != pm->idle_task) {
        curr = curr->next_proc;
    }

    curr->next_proc = proc;
    proc->next_proc = pm->idle_task;

    return 0;
}

int pm_remove_proc(proc_man_t * pm, process_t * proc) {
    // TODO not yet implemented, because idk how this struct will be
    return -1;
}
