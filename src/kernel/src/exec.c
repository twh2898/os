#include "exec.h"

#include "cpu/mmu.h"
#include "cpu/ram.h"
#include "libc/stdio.h"
#include "libc/string.h"

typedef int (*ff_t)(size_t argc, char ** argv);

int command_exec(uint8_t * buff, size_t size, size_t argc, char ** argv) {
    void * app_table_paddr = ram_page_alloc();
    // mmu_page_dir_t * dir = mmu_get_curr_dir();
    mmu_page_dir_t * dir = UINT2PTR(VADDR_PAGE_DIR);

    mmu_dir_set(dir, 1, app_table_paddr, MMU_DIR_RW);

    mmu_page_table_t * last_table = mmu_dir_get_table(dir, PAGE_DIR_SIZE - 1);
    mmu_table_set(last_table, 1, PTR2UINT(app_table_paddr), MMU_DIR_RW);

    mmu_page_table_t * app_table = mmu_dir_get_table(dir, 1);

    size_t page_count = ADDR2PAGE(PAGE_ALIGNED(size));
    for (size_t i = 0; i < page_count; i++) {
        void * page_paddr = ram_page_alloc();
        mmu_table_set(app_table, i, PTR2UINT(page_paddr), MMU_TABLE_RW);
    }

    kmemcpy(UINT2PTR(VADDR_FREE_MEM_USER), buff, size);

    ff_t call = UINT2PTR(VADDR_FREE_MEM_USER);

    int res = call(argc, argv);

    for (size_t i = 0; i < page_count; i++) {
        void * page_paddr = UINT2PTR(mmu_table_get_addr(app_table, i));
        ram_page_free(page_paddr);
        mmu_table_set(app_table, i, 0, 0);
    }

    mmu_dir_set(dir, 1, 0, 0);
    ram_page_free(app_table_paddr);

    return res;
}
