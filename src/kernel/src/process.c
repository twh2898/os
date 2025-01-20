#include "process.h"

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

    proc->pid              = next_pid();
    proc->next_heap_page   = ADDR2PAGE(VADDR_USER_MEM);
    proc->stack_page_count = 1;

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

    uint32_t kernel_table_addr = mmu_dir_get_addr((mmu_dir_t *)VADDR_KERNEL_DIR, 0);

    // Copy first page table from kernel page directory
    mmu_dir_clear(dir);
    mmu_dir_set(dir, 0, kernel_table_addr, MMU_DIR_RW);

    // Setup stack table
    uint32_t table_addr = ram_page_alloc();

    if (!table_addr) {
        paging_temp_free(proc->cr3);
        ram_page_free(proc->cr3);
        return -1;
    }

    mmu_dir_set(dir, MMU_DIR_SIZE - 1, table_addr, MMU_DIR_RW);

    proc->esp  = VADDR_USER_STACK;
    proc->esp0 = VADDR_ISR_STACK;

    // Allocate pages for ISR stack
    if (paging_add_pages(dir, ADDR2PAGE(proc->esp + 1), ADDR2PAGE(proc->esp0))) {
        paging_temp_free(proc->cr3);
        ram_page_free(proc->cr3);
        ram_page_free(table_addr);
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

    // Free tables, skip first (kernel)
    for (size_t i = 1; i < MMU_DIR_SIZE; i++) {
        if (!(mmu_dir_get_flags(dir, i) & MMU_DIR_FLAG_PRESENT)) {
            continue;
        }

        uint32_t      table_addr = mmu_dir_get_addr(dir, i);
        mmu_table_t * table      = paging_temp_map(table_addr);

        if (!table) {
            paging_temp_free(proc->cr3);
            ram_page_free(proc->cr3);
            return -1;
        }

        // Free pages
        for (size_t j = 0; j < MMU_TABLE_SIZE; j++) {
            if (mmu_table_get_flags(table, j) & MMU_TABLE_FLAG_PRESENT) {
                uint32_t page_addr = mmu_table_get_addr(table, j);
                ram_page_free(page_addr);
            }
        }

        paging_temp_free(table_addr);
        ram_page_free(table_addr);
    }

    // Free dir
    paging_temp_free(proc->cr3);
    ram_page_free(proc->cr3);

    return 0;
}

void * process_add_pages(process_t * proc, size_t count) {
    if (!proc || !count) {
        return 0;
    }

    if (proc->next_heap_page + count >= MMU_DIR_SIZE * MMU_TABLE_SIZE) {
        return 0;
    }

    mmu_dir_t * dir = paging_temp_map(proc->cr3);

    if (!dir) {
        return 0;
    }

    if (paging_add_pages(dir, proc->next_heap_page, proc->next_heap_page + count)) {
        paging_temp_free(proc->cr3);
        return 0;
    }

    paging_temp_free(proc->cr3);

    void * ptr = UINT2PTR(PAGE2ADDR(proc->next_heap_page));
    proc->next_heap_page += count;

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

    if (paging_add_pages(dir, new_stack_page_i, new_stack_page_i)) {
        paging_temp_free(proc->cr3);
        return -1;
    }

    proc->stack_page_count--;

    paging_temp_free(proc->cr3);

    return 0;
}

static uint32_t __pid;

static uint32_t next_pid() {
    static int pid_set = 0;
    if (!pid_set) {
        __pid   = 1;
        pid_set = 1;
    }
    return __pid++;
}

void set_next_pid(uint32_t next) {
    next_pid(); // Force pid_set to true so it doesn't override this value
    __pid = next;
}
