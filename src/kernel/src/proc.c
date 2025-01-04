#include "proc.h"

#include "cpu/mmu.h"
#include "libc/string.h"
#include "memory.h"
#include "paging.h"
#include "ram.h"

static uint32_t make_user_page_dir();

process_t * proc_new(void * fn, uint32_t ss0) {
    process_t * proc = impl_kmalloc(sizeof(process_t));
    if (!proc) {
        return 0;
    }

    mmu_page_dir_t * curr_dir       = mmu_get_curr_dir();
    uint32_t         proc_dir_paddr = make_user_page_dir();

    // TODO setup stack

    proc->eip = PTR2UINT(fn);
    // proc->esp       = esp;
    proc->cr3       = proc_dir_paddr;
    proc->ss0       = ss0;
    proc->pid       = 0;
    proc->pages     = arr_new(3, sizeof(void *));
    proc->next_proc = 0;
    return proc;
}

process_t * proc_from_ptr(uint32_t eip, uint32_t cr3, uint32_t esp, uint32_t ss0) {
    process_t * proc = impl_kmalloc(sizeof(process_t));
    if (proc) {
        proc->eip       = eip;
        proc->esp       = esp;
        proc->cr3       = cr3;
        proc->ss0       = ss0;
        proc->pid       = 0;
        proc->pages     = arr_new(3, sizeof(void *));
        proc->next_proc = 0;
    }
    return proc;
}

void proc_free(process_t * proc) {
    if (proc) {
        for (size_t i = 0; i < arr_size(proc->pages); i++) {
            void * paddr;
            arr_get(proc->pages, i, &paddr);
            ram_page_free(PTR2UINT(paddr));
        }
        arr_free(proc->pages);
        impl_kfree(proc);
    }
}

proc_man_t * proc_man_new() {
    proc_man_t * pm = impl_kmalloc(sizeof(proc_man_t));
    if (!pm) {
        return 0;
    }

    pm->idle_task  = 0;
    pm->task_begin = 0;
    pm->curr_task  = 0;

    return pm;
}

void proc_man_set_idle(proc_man_t * pm, process_t * proc) {
}

void proc_man_set_new_idle(proc_man_t * pm, uint32_t eip, uint32_t esp, uint32_t cr3, uint32_t ss0) {
    if (!pm || !eip || !esp || !cr3) {
        return;
    }

    process_t * idle_proc = impl_kmalloc(sizeof(process_t));
    idle_proc->eip        = eip;
    idle_proc->esp        = esp;
    idle_proc->cr3        = cr3;
    idle_proc->ss0        = ss0;

    pm->idle_task = idle_proc;
}

int proc_man_task_from_ptr(void * fn) {
    process_t * new_proc = impl_kmalloc(sizeof(process_t));
}

void proc_man_switch_to_idle(proc_man_t * pm) {
}

static void stack_pages(mmu_page_dir_t * dir, size_t n) {
    // Page table for stack
    uint32_t stack_table_page = ram_page_alloc();
    mmu_dir_set(dir, PAGE_DIR_SIZE - 2, stack_table_page, MMU_DIR_RW);
    mmu_page_table_t * stack_table = mmu_table_create(UINT2PTR(stack_table_page));

    // First stack page
    uint32_t stack_page = ram_page_alloc();
    mmu_table_set(stack_table, PAGE_TABLE_SIZE - 1, stack_page, MMU_TABLE_RW);
}

static uint32_t make_user_page_dir() {
    uint32_t         new_dir_page = ram_page_alloc();
    mmu_page_dir_t * dir          = mmu_dir_create(paging_temp_map(new_dir_page));
    paging_temp_free(new_dir_page);
    return new_dir_page;
}
