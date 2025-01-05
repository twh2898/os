#include "proc.h"

#include "cpu/mmu.h"
#include "libc/string.h"
#include "memory.h"
#include "paging.h"
#include "ram.h"

static uint32_t make_user_page_dir();

void process_create(process_t * proc) {
    static uint32_t next_pid = 1;

    kmemset(proc, 0, sizeof(process_t));

    proc->pid       = next_pid++;
    proc->next_page = VADDR_FREE_MEM_USER;

    // Setup page directory
    {
        uint32_t addr = ram_page_alloc();

        if (!addr) {
            return;
        }

        mmu_dir_t * dir = paging_temp_map(addr);

        if (!dir) {
            ram_page_free(addr);
            return;
        }

        // Copy first page table from kernel page directory
        mmu_dir_t * kernel_dir = (mmu_dir_t *)VADDR_PAGE_DIR;
        mmu_dir_clear(dir);
        mmu_dir_set(dir, 0, mmu_dir_get_addr(kernel_dir, 0), MMU_DIR_RW);

        // TODO setup stack

        paging_temp_free(addr);
    }

    // TODO setup stack
}

int process_add_pages(process_t * proc, size_t count) {
    if (!proc || !count) {
        return -1;
    }

    for (size_t i = 0; i < count; i++) {
        uint32_t addr = ram_page_alloc();

        if (!addr) {
            return -1;
        }

        uint32_t page_i  = proc->next_page;
        uint32_t dir_i   = page_i / MMU_DIR_SIZE;
        uint32_t table_i = page_i % MMU_DIR_SIZE;

        mmu_dir_t * dir = paging_temp_map(proc->cr3);

        if (!dir) {
            ram_page_free(addr);
            return -1;
        }

        mmu_table_t * table = paging_temp_map(mmu_dir_get_addr(dir, dir_i));

        if (!table) {
            paging_temp_free(PTR2UINT(table));
            ram_page_free(addr);
            return -1;
        }

        mmu_table_set(table, table_i, addr, MMU_TABLE_RW);

        paging_temp_free(PTR2UINT(table));
        paging_temp_free(PTR2UINT(dir));
    }

    return 0;
}

process_t * proc_new(void * fn, uint32_t ss0) {
    process_t * proc = impl_kmalloc(sizeof(process_t));
    if (!proc) {
        return 0;
    }

    mmu_dir_t * curr_dir       = mmu_get_curr_dir();
    uint32_t    proc_dir_paddr = make_user_page_dir();

    // TODO setup stack

    // proc->eip = PTR2UINT(fn);
    // proc->esp       = esp;
    proc->cr3 = proc_dir_paddr;
    proc->ss0 = ss0;
    proc->pid = 0;
    // proc->pages     = arr_new(3, sizeof(void *));
    proc->next_proc = 0;
    return proc;
}

process_t * proc_from_ptr(uint32_t eip, uint32_t cr3, uint32_t esp, uint32_t ss0) {
    process_t * proc = impl_kmalloc(sizeof(process_t));
    if (proc) {
        // proc->eip       = eip;
        proc->esp = esp;
        proc->cr3 = cr3;
        proc->ss0 = ss0;
        proc->pid = 0;
        // proc->pages     = arr_new(3, sizeof(void *));
        proc->next_proc = 0;
    }
    return proc;
}

void proc_free(process_t * proc) {
    // if (proc) {
    //     for (size_t i = 0; i < arr_size(proc->pages); i++) {
    //         void * paddr;
    //         arr_get(proc->pages, i, &paddr);
    //         ram_page_free(PTR2UINT(paddr));
    //     }
    //     arr_free(proc->pages);
    //     impl_kfree(proc);
    // }
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
    // idle_proc->eip        = eip;
    idle_proc->esp = esp;
    idle_proc->cr3 = cr3;
    idle_proc->ss0 = ss0;

    pm->idle_task = idle_proc;
}

int proc_man_task_from_ptr(void * fn) {
    process_t * new_proc = impl_kmalloc(sizeof(process_t));
}

void proc_man_switch_to_idle(proc_man_t * pm) {
}

static void stack_pages(mmu_dir_t * dir, size_t n) {
    // Page table for stack
    uint32_t stack_table_page = ram_page_alloc();
    mmu_dir_set(dir, MMU_DIR_SIZE - 2, stack_table_page, MMU_DIR_RW);
    mmu_table_t * stack_table = UINT2PTR(stack_table_page);
    mmu_table_clear(stack_table);

    // First stack page
    uint32_t stack_page = ram_page_alloc();
    mmu_table_set(stack_table, MMU_TABLE_SIZE - 1, stack_page, MMU_TABLE_RW);
}

static uint32_t make_user_page_dir() {
    uint32_t    new_dir_page = ram_page_alloc();
    mmu_dir_t * dir          = paging_temp_map(new_dir_page);
    mmu_dir_clear(dir);
    paging_temp_free(new_dir_page);
    return new_dir_page;
}
