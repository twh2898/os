#include "process_manager.h"

#include "libc/proc.h"
#include "libc/stdio.h"

static int pid_arr_index(arr_t * arr, int pid);

int pm_create(proc_man_t * pm) {
    if (!pm) {
        return -1;
    }

    if (arr_create(&pm->task_list, 4, sizeof(process_t *))) {
        return -1;
    }

    return 0;
}

process_t * pm_get_active(proc_man_t * pm) {
    if (!pm) {
        return 0;
    }

    // TODO this is redundant now
    return get_active_task();
}

process_t * pm_find_pid(proc_man_t * pm, int pid) {
    if (!pm || pid < 1) {
        return 0;
    }

    int i = pid_arr_index(&pm->task_list, pid);
    if (i < 0) {
        return 0;
    }

    process_t * proc;
    arr_get(&pm->task_list, i, &proc);

    return proc;
}

int pm_add_proc(proc_man_t * pm, process_t * proc) {
    if (!pm || !proc) {
        return -1;
    }

    if (arr_insert(&pm->task_list, arr_size(&pm->task_list), &proc)) {
        return -1;
    }

    return 0;
}

int pm_remove_proc(proc_man_t * pm, int pid) {
    if (!pm || pid < 1) {
        return -1;
    }

    if (pid == get_active_task()->pid) {
        return -1;
    }

    int i = pid_arr_index(&pm->task_list, pid);
    if (i < 0 || arr_remove(&pm->task_list, i, 0)) {
        return -1;
    }

    return -1;
}

int pm_resume_process(proc_man_t * pm, int pid, ebus_event_t * event) {
    if (!pm) {
        return -1;
    }

    process_t * proc = pm_find_pid(pm, pid);
    if (!proc) {
        return -1;
    }

    return process_resume(proc, event);
}

process_t * pm_get_next(proc_man_t * pm) {
    if (!pm) {
        return 0;
    }

    int i = pid_arr_index(&pm->task_list, get_active_task()->pid);
    if (i < 0) {
        return 0;
    }

    i++;

    if (i >= arr_size(&pm->task_list)) {
        i = 0;
    }

    process_t * proc;
    if (arr_get(&pm->task_list, i, &proc)) {
        return 0;
    }

    return proc;
}

static int pid_arr_index(arr_t * arr, int pid) {
    if (!arr) {
        return -1;
    }

    for (int i = 0; i < arr_size(arr); i++) {
        process_t * proc;
        arr_get(arr, i, &proc);

        if (proc->pid == pid) {
            return i;
        }
    }

    return -1;
}

int pm_push_event(proc_man_t * pm, ebus_event_t * event) {
    if (!pm || !event) {
        return -1;
    }

    printf("Task list size is %u\n", pm->task_list.size);

    for (size_t i = 0; i < arr_size(&pm->task_list); i++) {
        process_t * proc;
        arr_get(&pm->task_list, i, &proc);

        if (proc->state >= PROCESS_STATE_DEAD) {
            continue;
        }

        ebus_push(&proc->event_queue, event);
        printf("Proc %u size is %u\n", proc->pid, proc->event_queue.queue.len);
        if (proc->filter_event && event && event->event_id == proc->filter_event) {
            proc->state = PROCESS_STATE_SUSPENDED;
        }
    }

    return 0;
}
