#include "process.h"

#include "cpu/mmu.h"
#include "cpu/tss.h"
#include "kernel.h"
#include "libc/string.h"
#include "libk/sys_call.h"
#include "paging.h"
#include "ram.h"

static uint32_t next_pid();

int process_create(process_t * proc) {
    if (!proc) {
        return -1;
    }

    kmemset(proc, 0, sizeof(process_t));

    proc->cr3 = ram_page_alloc();

    if (!proc->cr3) {
        return -1;
    }

    if (arr_create(&proc->io_handles, 1, sizeof(handle_t))) {
        ram_page_free(proc->cr3);
        return -1;
    }

    if (ebus_create(&proc->event_queue, 4096)) {
        arr_free(&proc->io_handles);
        ram_page_free(proc->cr3);
        return -1;
    }

    if (memory_init(&proc->memory, _sys_page_alloc)) {
        ebus_free(&proc->event_queue);
        arr_free(&proc->io_handles);
        ram_page_free(proc->cr3);
        return -1;
    }

    // Setup page directory
    mmu_dir_t * dir = paging_temp_map(proc->cr3);

    if (!dir) {
        ebus_free(&proc->event_queue);
        arr_free(&proc->io_handles);
        ram_page_free(proc->cr3);
        return -1;
    }

    uint32_t kernel_table_addr = mmu_dir_get_addr((mmu_dir_t *)VADDR_KERNEL_DIR, 0);

    // Copy first page table from kernel page directory
    mmu_dir_clear(dir);
    mmu_dir_set(dir, 0, kernel_table_addr, MMU_DIR_RW);

    proc->esp  = VADDR_USER_STACK;
    proc->esp0 = VADDR_ISR_STACK;

    // Allocate pages for ISR stack + first page of user stack
    if (paging_add_pages(dir, ADDR2PAGE(proc->esp), ADDR2PAGE(proc->esp0))) {
        ebus_free(&proc->event_queue);
        arr_free(&proc->io_handles);
        paging_temp_free(proc->cr3);
        ram_page_free(proc->cr3);
        return -1;
    }

    proc->pid              = next_pid();
    proc->next_heap_page   = ADDR2PAGE(VADDR_USER_MEM);
    proc->stack_page_count = 1;

    paging_temp_free(proc->cr3);

    return 0;
}

int process_free(process_t * proc) {
    if (!proc) {
        return -1;
    }

    ebus_free(&proc->event_queue);
    arr_free(&proc->io_handles);

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

int process_set_entrypoint(process_t * proc, void * entrypoint) {
    if (!proc || !entrypoint || proc->state >= PROCESS_STATE_SUSPENDED) {
        return -1;
    }

    uint32_t ret_addr = proc->esp;
    uint32_t ret_page = ADDR2PAGE(ret_addr);
    uint32_t dir_i    = ret_page / MMU_DIR_SIZE;
    uint32_t table_i  = ret_page % MMU_TABLE_SIZE;

    mmu_dir_t * dir = paging_temp_map(proc->cr3);

    if (!dir) {
        return -1;
    }

    uint32_t table_addr = mmu_dir_get_addr(dir, dir_i);

    mmu_table_t * table = paging_temp_map(table_addr);

    if (!table) {
        paging_temp_free(proc->cr3);
        return -1;
    }

    uint32_t page_addr = mmu_table_get_addr(table, table_i);

    uint32_t * stack = paging_temp_map(page_addr);

    int ret_i    = (proc->esp % PAGE_SIZE) / 4;
    stack[ret_i] = PTR2UINT(entrypoint);

    if (!stack) {
        paging_temp_free(table_addr);
        paging_temp_free(proc->cr3);
        return -1;
    }

    paging_temp_free(page_addr);
    paging_temp_free(table_addr);
    paging_temp_free(proc->cr3);

    proc->esp -= (5 * 4) - 1;

    return 0;
}

// int process_activate(process_t * proc) {
//     if (!proc) {
//         return -1;
//     }

//     tss_set_esp0(proc->esp0); // just updates tss 0
//     mmu_change_dir(proc->cr3);

//     return 0;
// }

// int process_yield(process_t * proc, uint32_t esp, uint32_t eip, int filter) {
//     if (!proc || !esp || !eip) {
//         return -1;
//     }

//     proc->eip          = eip;
//     proc->filter_event = filter;
//     proc->esp          = esp;
//     proc->state        = PROCESS_STATE_SUSPENDED;

//     return 0;
// }

int process_resume(process_t * proc, const ebus_event_t * event) {
    if (!proc || proc->state < PROCESS_STATE_LOADED || proc->state >= PROCESS_STATE_DEAD) {
        return -1;
    }

    process_t * active_before = get_active_task();
    active_before->state      = PROCESS_STATE_SUSPENDED;

    proc->state = PROCESS_STATE_RUNNING;
    switch_task(proc);

    // Call this again because we are a new process now
    process_t * active_after = get_active_task();
    active_after->state      = PROCESS_STATE_RUNNING;

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

int process_load_heap(process_t * proc, const char * buff, size_t size) {
    if (!proc || !buff || !size) {
        return -1;
    }

    proc->state = PROCESS_STATE_LOADING;

    size_t page_count = ADDR2PAGE(size);
    if (size & MASK_FLAGS) {
        page_count++;
    }

    uint32_t heap_start = proc->next_heap_page;
    void *   heap_alloc = process_add_pages(proc, page_count);

    if (!heap_alloc) {
        return -1;
    }

    mmu_dir_t * dir = paging_temp_map(proc->cr3);

    if (!dir) {
        return -1;
    }

    for (size_t i = 0; i < page_count; i++) {
        uint32_t      table_addr = mmu_dir_get_addr(dir, (heap_start + i) / MMU_TABLE_SIZE);
        mmu_table_t * table      = paging_temp_map(table_addr);

        if (!table) {
            paging_temp_free(proc->cr3);
            return -1;
        }

        uint32_t addr     = mmu_table_get_addr(table, (heap_start + i) % MMU_TABLE_SIZE);
        void *   tmp_page = paging_temp_map(addr);

        if (!tmp_page) {
            paging_temp_free(table_addr);
            paging_temp_free(proc->cr3);
            return -1;
        }

        size_t to_copy = PAGE_SIZE;

        if (i == page_count - 1) {
            to_copy = size % PAGE_SIZE;
        }

        kmemcpy(tmp_page, &buff[i * PAGE_SIZE], to_copy);

        paging_temp_free(addr);
        paging_temp_free(table_addr);
    }

    paging_temp_free(proc->cr3);

    proc->state = PROCESS_STATE_LOADED;

    return 0;
}

static uint32_t __pid;

static uint32_t next_pid() {
    static int pid_set = 0;
    // Handle initializing __pid because there is no static init
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
