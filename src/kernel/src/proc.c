#include "proc.h"

#include "cpu/mmu.h"
#include "libc/string.h"
#include "paging.h"
#include "ram.h"

static uint32_t next_pid();

int process_create(process_t * proc) {
    if (!proc) {
        return -1;
    }

    kmemset(proc, 0, sizeof(process_t));

    proc->pid            = next_pid();
    proc->next_heap_page = VADDR_USER_MEM;

    proc->cr3 = ram_page_alloc();

    if (!proc->cr3) {
        return -1;
    }

    // Setup page directory
    mmu_dir_t * dir = paging_temp_map(proc->cr3);

    if (!dir) {
        ram_page_free(proc->cr3);
        return -1;
    }

    // Copy first page table from kernel page directory
    mmu_dir_clear(dir);
    mmu_dir_set(dir, 0, VADDR_KERNEL_TABLE, MMU_DIR_RW);

    // Setup stack table
    uint32_t table_addr = ram_page_alloc();

    if (!table_addr) {
        paging_temp_free(proc->cr3);
        ram_page_free(proc->cr3);
        return -1;
    }

    mmu_dir_set(dir, MMU_DIR_SIZE - 1, table_addr, MMU_DIR_RW);

    proc->esp              = VADDR_USER_STACK;
    proc->esp0             = proc->esp;
    proc->stack_page_count = 0;

    if (process_grow_stack(proc)) {
        paging_temp_free(proc->cr3);
        ram_page_free(proc->cr3);
        return -1;
    }

    paging_temp_free(proc->cr3);

    return 0;
}

int process_free(process_t * proc) {
    if (!proc) {
        return -1;
    }

    mmu_dir_t * dir = paging_temp_map(proc->cr3);

    if (!dir) {
        return -1;
    }

    // Free ram pages, skip first table (kernel) and last table (tables)
    for (size_t i = 1; i < MMU_DIR_SIZE - 1; i++) {
        if (!(mmu_dir_get_flags(dir, i) & MMU_DIR_FLAG_PRESENT)) {
            continue;
        }

        uint32_t      table_addr = mmu_dir_get_addr(dir, i);
        mmu_table_t * table      = paging_temp_map(table_addr);

        if (!table) {
            paging_temp_free(proc->cr3);
            return -1;
        }

        for (size_t j = 0; j < MMU_TABLE_SIZE; j++) {
            if (mmu_table_get_flags(table, j) & MMU_TABLE_FLAG_PRESENT) {
                uint32_t page_addr = mmu_table_get_addr(table, j);
                ram_page_free(page_addr);
            }
        }

        paging_temp_free(table_addr);
        ram_page_free(table_addr);
    }

    paging_temp_free(proc->cr3);
    ram_page_free(proc->cr3);

    return 0;
}

void * process_add_pages(process_t * proc, size_t count) {
    if (!proc || !count) {
        return 0;
    }

    void * ptr = UINT2PTR(PAGE2ADDR(proc->next_heap_page));

    for (size_t i = 0; i < count; i++) {
        uint32_t addr = ram_page_alloc();

        if (!addr) {
            return 0;
        }

        uint32_t page_i  = proc->next_heap_page;
        uint32_t dir_i   = page_i / MMU_DIR_SIZE;
        uint32_t table_i = page_i % MMU_TABLE_SIZE;

        mmu_dir_t * dir = paging_temp_map(proc->cr3);

        if (!dir) {
            ram_page_free(addr);
            return 0;
        }

        mmu_table_t * table = paging_temp_map(mmu_dir_get_addr(dir, dir_i));

        if (!table) {
            paging_temp_free(PTR2UINT(table));
            ram_page_free(addr);
            return 0;
        }

        mmu_table_set(table, table_i, addr, MMU_TABLE_RW);

        proc->next_heap_page++;

        paging_temp_free(PTR2UINT(table));
        paging_temp_free(PTR2UINT(dir));
    }

    return ptr;
}

int process_grow_stack(process_t * proc) {
    if (!proc) {
        return -1;
    }

    mmu_dir_t * dir = paging_temp_map(proc->cr3);

    if (!dir) {
        return -1;
    }

    size_t new_stack_page_i = MMU_DIR_SIZE * MMU_TABLE_SIZE - proc->stack_page_count - 1;

    size_t dir_i   = new_stack_page_i / MMU_DIR_SIZE;
    size_t table_i = new_stack_page_i % MMU_TABLE_SIZE;

    // Need new table
    if (!(mmu_dir_get_flags(dir, dir_i) & MMU_DIR_FLAG_PRESENT)) {
        uint32_t addr = ram_page_alloc();

        if (!addr) {
            paging_temp_free(proc->cr3);
            return -1;
        }

        mmu_dir_set(dir, dir_i, addr, MMU_DIR_RW);
    }

    uint32_t      table_addr = mmu_dir_get_addr(dir, dir_i);
    mmu_table_t * table      = paging_temp_map(table_addr);

    if (!table) {
        paging_temp_free(proc->cr3);
        return -1;
    }

    uint32_t addr = ram_page_alloc();

    if (!addr) {
        paging_temp_free(table_addr);
        paging_temp_free(proc->cr3);
        return -1;
    }

    mmu_table_set(table, table_i, addr, MMU_TABLE_RW);

    proc->stack_page_count++;

    paging_temp_free(table_addr);
    paging_temp_free(proc->cr3);

    return 0;
}

// Old

// proc_man_t * proc_man_new() {
//     proc_man_t * pm = impl_kmalloc(sizeof(proc_man_t));
//     if (!pm) {
//         return 0;
//     }

//     pm->idle_task  = 0;
//     pm->task_begin = 0;
//     pm->curr_task  = 0;

//     return pm;
// }

// void proc_man_set_idle(proc_man_t * pm, process_t * proc) {
// }

// void proc_man_set_new_idle(proc_man_t * pm, uint32_t eip, uint32_t esp, uint32_t cr3, uint32_t ss0) {
//     if (!pm || !eip || !esp || !cr3) {
//         return;
//     }

//     process_t * idle_proc = impl_kmalloc(sizeof(process_t));
//     // idle_proc->eip        = eip;
//     idle_proc->esp = esp;
//     idle_proc->cr3 = cr3;
//     idle_proc->ss0 = ss0;

//     pm->idle_task = idle_proc;
// }

// int proc_man_task_from_ptr(void * fn) {
//     process_t * new_proc = impl_kmalloc(sizeof(process_t));
// }

// void proc_man_switch_to_idle(proc_man_t * pm) {
// }

// static void stack_pages(mmu_dir_t * dir, size_t n) {
//     // Page table for stack
//     uint32_t stack_table_page = ram_page_alloc();
//     mmu_dir_set(dir, MMU_DIR_SIZE - 2, stack_table_page, MMU_DIR_RW);
//     mmu_table_t * stack_table = UINT2PTR(stack_table_page);
//     mmu_table_clear(stack_table);

//     // First stack page
//     uint32_t stack_page = ram_page_alloc();
//     mmu_table_set(stack_table, MMU_TABLE_SIZE - 1, stack_page, MMU_TABLE_RW);
// }

static uint32_t next_pid() {
    static uint32_t pid = 1; // 0 is reserved for kernel
    return pid++;
}
