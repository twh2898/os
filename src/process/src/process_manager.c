#include "process_manager.h"

#include "libc/proc.h"
#include "libc/stdio.h"

int pm_create(proc_man_t * pm) {
    if (!pm) {
        return -1;
    }

    if (arr_create(&pm->task_list, 4, sizeof(process_t *))) {
        return -1;
    }

    if (arr_create(&pm->waiting, 4, sizeof(process_t *))) {
        arr_free(&pm->task_list);
        return -1;
    }

    pm->active = 0;

    return 0;
}

process_t * pm_get_pid(proc_man_t * pm, int pid) {
    if (!pm || pid < 1) {
        return 0;
    }

    for (size_t i = 0; i < arr_size(&pm->task_list); i++) {
        process_t * p;
        arr_get(&pm->task_list, i, &p);

        if (p->pid == pid) {
            return p;
        }
    }

    return 0;
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
        return 0;
    }

    if (pid == pm->active->pid) {
        return -1;
    }

    for (size_t i = 0; i < arr_size(&pm->waiting); i++) {
        process_t * p;
        arr_get(&pm->waiting, i, &p);

        if (p->pid == pid) {
            if (arr_remove(&pm->waiting, i, 0)) {
                return -1;
            }
            break;
        }
    }

    for (size_t i = 0; i < arr_size(&pm->task_list); i++) {
        process_t * p;
        arr_get(&pm->task_list, i, &p);

        if (p->pid == pid) {
            if (arr_remove(&pm->task_list, i, 0)) {
                return -1;
            }
            process_free(p);
            return 0;
        }
    }

    return -1;
}

static void handle_launch(const ebus_event_t * event) {
    printf("Start task %u\n", event->task_switch.next_task_pid);
    process_t * proc      = pm_get_pid(&__kernel.pm, event->task_switch.next_task_pid);
    __kernel.pm.curr_task = proc;
    process_resume(proc, 0);
}

static void handle_kill(const ebus_event_t * event) {
    printf("Kill task %u\n", event->task_switch.next_task_pid);
    process_t * proc = pm_get_pid(&__kernel.pm, event->task_kill.task_pid);
    if (pm_remove_proc(&__kernel.pm, proc)) {
        PANIC("Failed to remove process from pm");
    }

    process_free(proc);
}

int kernel_call_as_proc(int pid, _proc_call_t fn, void * data) {
    if (!fn) {
        return -1;
    }

    process_t * curr = get_current_process();
    if (curr->pid != pid) {
        kernel_switch_task(pid);
    }

    int res = fn(data);

    if (curr->pid != pid) {
        kernel_switch_task(curr->pid);
    }

    return res;
}

int kernel_switch_task(int next_pid) {
    process_t * proc = pm_get_pid(&__kernel.pm, next_pid);
    if (!proc || proc->state == PROCESS_STATE_DEAD) {
        return -1;
    }

    __kernel.pm.curr_task = proc;
    proc->state           = PROCESS_STATE_RUNNING;
    set_kernel_stack(proc->esp0);
    mmu_change_dir(proc->cr3);
    return 0;
}

process_t * get_current_process() {
    return __kernel.pm.curr_task;
}

int kernel_add_task(process_t * proc) {
    if (!proc) {
        return -1;
    }
    process_t * curr = __kernel.pm.task_begin;
    while (curr->next_proc && curr->next_proc != __kernel.pm.idle_task) {
        curr = curr->next_proc;
    }
    curr->next_proc = proc;
    proc->next_proc = __kernel.pm.idle_task;
    return 0;
}

int kernel_next_task() {
    process_t * curr = get_current_process();
    process_t * next = curr->next_proc;
    if (!next) {
        next = __kernel.pm.idle_task;
    }

    __kernel.pm.curr_task = next;

    // TODO interrupt back to process for sigint

    if (process_resume(__kernel.pm.curr_task, 0)) {
        KPANIC("Failed to resume task");
    }

    return 0;
}

int kernel_close_process(process_t * proc) {
    if (!proc) {
        return -1;
    }

    proc->state = PROCESS_STATE_DEAD;

    process_t * next = __kernel.pm.curr_task->next_proc;
    if (!next) {
        next = __kernel.pm.idle_task;
    }

    ebus_event_t launch_event;
    launch_event.event_id                  = EBUS_EVENT_TASK_SWITCH;
    launch_event.task_switch.next_task_pid = next->pid;

    queue_event(&launch_event);

    ebus_event_t kill_event;
    kill_event.event_id           = EBUS_EVENT_TASK_KILL;
    kill_event.task_kill.task_pid = proc->pid;

    queue_event(&kill_event);

    return 0;
}
