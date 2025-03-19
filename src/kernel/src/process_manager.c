#include "process_manager.h"

#include "libc/stdio.h"

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
        KPANIC("Failed to remove process from pm");
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
