#ifndef KERNEL_H
#define KERNEL_H

#include <stddef.h>
#include <stdint.h>

#include "cpu/mmu.h"
#include "ebus.h"
#include "memory_alloc.h"
#include "process.h"
#include "process_manager.h"

typedef struct _kernel {
    uint32_t   ram_table_addr;
    uint32_t   cr3;
    process_t  proc;
    proc_man_t pm;
    memory_t   kernel_memory;
    ebus_t     event_bus;
} kernel_t;

/**
 * @brief Get a pointer to the kernel's page directory.
 *
 * This address is identity mapped and should be the same for virtual and
 * physical address spaces.
 *
 * @return mmu_dir_t* pointer to the kernel's page directory
 */
mmu_dir_t * get_kernel_dir();

/**
 * @brief Get a pointer to the virtual address of the first page table.
 *
 * The first page table is the kernel's memory space. This pointer is the
 * virtual address of the table within the kernel's memory space.
 *
 * @return mmu_table_t* pointer to the kernel's page table of any page directory
 */
mmu_table_t * get_kernel_table();

process_t * get_current_process();

ebus_t * get_kernel_ebus();

void tmp_register_signals_cb(signals_master_cb_t cb);

ebus_event_t * pull_event(int event_id);

int kernel_add_task(process_t * proc);
int kernel_next_task();
int kernel_close_process(process_t * proc);
int kernel_set_current_task(process_t * proc);

typedef int (*_proc_call_t)(void * data);

int kernel_call_as_proc(int pid, _proc_call_t fn, void * data);

int kernel_switch_task(int next_pid);

#ifdef TESTING
#define NO_RETURN
#else
#define NO_RETURN _Noreturn
#endif

#define KPANIC(MSG) kernel_panic((MSG), __FILE__, __LINE__)
NO_RETURN void kernel_panic(const char * msg, const char * file, unsigned int line);

#endif // KERNEL_H
