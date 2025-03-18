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

process_t * pm_get_pid(proc_man_t * pm, int pid) {
    if (!pm || pid < 1) {
        return 0;
    }

    if (pm->idle_task->pid == pid) {
        return pm->idle_task;
    }

    if (pm->curr_task->pid == pid) {
        return pm->curr_task;
    }

    process_t * curr = pm->task_begin;
    while (curr && curr->pid != pm->idle_task->pid) {
        if (curr->pid == pid) {
            return curr;
        }
        curr = curr->next_proc;
    }

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
    if (!pm || !proc) {
        return -1;
    }

    if (!proc->pid || proc->pid == pm->idle_task->pid) {
        return -1;
    }

    if (pm->curr_task->pid == proc->pid) {
        pm->curr_task = pm->curr_task->next_proc;
    }

    // TODO not yet implemented, because idk how this struct will be
    process_t * curr = pm->task_begin;

    if (curr->pid == proc->pid) {
        pm->task_begin = curr->next_proc;
        return 0;
    }

    while (curr->next_proc && curr->next_proc != pm->idle_task && curr->next_proc != pm->curr_task) {
        if (curr->next_proc->pid == proc->pid) {
            curr->next_proc = curr->next_proc->next_proc;
            return 0;
        }
        curr = curr->next_proc;
    }

    return -1;
}
