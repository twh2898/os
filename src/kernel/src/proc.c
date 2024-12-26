#include "proc.h"

#include "cpu/mmu.h"
#include "cpu/ram.h"
#include "libc/string.h"
#include "memory.h"

static mmu_page_dir_t * make_user_page_dir(mmu_page_dir_t *);
void *                  tmp_map(void * paddr, size_t i);

process_t * proc_new(void * fn, uint32_t ss0) {
    process_t * proc = impl_kmalloc(sizeof(process_t));
    if (!proc) {
        return 0;
    }

    mmu_page_dir_t * curr_dir       = mmu_get_curr_dir();
    void *           proc_dir_paddr = make_user_page_dir(curr_dir);

    // TODO setup stack

    proc->eip = PTR2UINT(fn);
    // proc->esp       = esp;
    proc->cr3       = PTR2UINT(proc_dir_paddr);
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
            ram_page_free(paddr);
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

static int id_map_range(mmu_page_table_t * table, size_t start, size_t end) {
    if (end > 1023) {
        return -1;
    }
    while (start <= end) {
        mmu_table_set(table, start, start << 12, MMU_TABLE_RW);
        start++;
    }
    return 0;
}

static mmu_page_dir_t * make_user_page_dir(mmu_page_dir_t * curr_dir) {
    void * new_dir_page   = ram_page_alloc();
    void * new_table_page = ram_page_alloc();

    mmu_page_dir_t * dir = mmu_dir_create(tmp_map(new_dir_page, 0));
    mmu_dir_set(dir, 0, new_table_page, MMU_DIR_RW);

    mmu_page_table_t * table_1 = mmu_table_create(tmp_map(new_table_page, 1));

    mmu_page_table_t * curr_table = mmu_dir_get_table(curr_dir, 0);

    mmu_table_set(table_1, 1, PTR2UINT(new_dir_page), MMU_TABLE_RW);
    id_map_range(table_1, 2, 0x9e);

    // TODO pages for stack

    return new_dir_page;
}

void * tmp_map(void * paddr, size_t i) {
    if (i >= VADDR_TMP_PAGE_COUNT) {
        return 0;
    }

    size_t table_i = ADDR2PAGE(VADDR_TMP_PAGE) + i;

    mmu_page_dir_t *   dir   = mmu_get_curr_dir();
    mmu_page_table_t * table = mmu_dir_get_table(dir, table_i);
    mmu_table_set(table, table_i, PTR2UINT(paddr), MMU_TABLE_RW_USER);
    return UINT2PTR(PAGE2ADDR(table_i));
}
