#ifndef KERNEL_PROCESS_H
#define KERNEL_PROCESS_H

#include <stddef.h>
#include <stdint.h>

typedef void (*signals_master_callback)(int);

typedef struct _process {
    uint32_t pid;
    uint32_t next_heap_page;
    uint32_t stack_page_count;

    // TODO heap & stack limits

    uint32_t cr3;
    uint32_t esp;
    uint32_t esp0;

    signals_master_callback signals_callback;

    struct _process * next_proc;
} process_t;

/**
 * @brief Create a new process and it's page directory.
 *
 * Allocates pages for the isr stack and 1 for the user stack.
 *
 * @param proc pointer to the process object
 * @return int 0 for success
 */
int process_create(process_t * proc);

/**
 * @brief Free pages used by `process` including it's page directory.
 *
 * This does not free the first table which is the kernel's table.
 *
 * @param proc pointer to the process object
 * @return int 0 for success
 */
int process_free(process_t * proc);

/**
 * @brief Add `count` pages to the process heap.
 *
 * @param proc pointer to the process object
 * @param count number of pages to add
 * @return pointer to the first new page in virtual memory
 */
void * process_add_pages(process_t * proc, size_t count);

/**
 * @brief Add a single page to expand the process stack
 *
 * @param proc pointer to the process object
 * @return int 0 for success
 */
int process_grow_stack(process_t * proc);

/**
 * @brief Set the next PID value. All future PID's will be incremented from
 * here.
 *
 * This can be used to reset, replace os skip certain PID values.
 *
 * @param next process id
 */
void set_next_pid(uint32_t next);

// Old

extern void set_first_task(process_t * next_proc);
extern void switch_to_task(process_t * next_proc);

typedef struct _proc_man {
    process_t * idle_task;
    process_t * task_begin;
    process_t * curr_task;
} proc_man_t;

#endif // KERNEL_PROCESS_H
