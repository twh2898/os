#include "exec.h"

#include "cpu/mmu.h"
#include "cpu/ram.h"
#include "libc/stdio.h"
#include "libc/string.h"

typedef int (*ff_t)(size_t argc, char ** argv);

static void setup_user_space(size_t size);
static void free_user_space();

int command_exec(uint8_t * buff, size_t size, size_t argc, char ** argv) {
    setup_user_space(size);

    kmemcpy(UINT2PTR(VADDR_FREE_MEM_USER), buff, size);

    ff_t call = UINT2PTR(VADDR_FREE_MEM_USER);

    int res = call(argc, argv);

    free_user_space();

    return res;
}

static void setup_user_space(size_t size) {
    mmu_page_dir_t * dir = UINT2PTR(VADDR_PAGE_DIR);
    mmu_page_table_t * last_table = UINT2PTR(VADDR_LAST_PAGE_TABLE);

    void * page = ram_page_alloc();
    mmu_dir_set(dir, 1, page, MMU_DIR_RW);
    mmu_table_set(last_table, 1, PTR2UINT(page), MMU_TABLE_RW);

    mmu_page_table_t * user_table = UINT2PTR(mmu_dir_get_vaddr(dir, 1));

    size_t page_count = PAGE_ALIGNED(size) >> 12;
    for (size_t i = 0; i < page_count; i++) {
        void * page_paddr = ram_page_alloc();
        mmu_table_set(user_table, i, PTR2UINT(page_paddr), MMU_TABLE_RW);
    }
}

static void free_user_space() {
    mmu_page_dir_t * dir = UINT2PTR(VADDR_PAGE_DIR);
    mmu_page_table_t * last_table = UINT2PTR(VADDR_LAST_PAGE_TABLE);

    mmu_page_table_t * user_table = UINT2PTR(mmu_dir_get_vaddr(dir, 1));

    for (size_t i = 0; i < PAGE_TABLE_SIZE; i++) {
        int flags = mmu_table_get_flags(user_table, i);
        if (flags & MMU_PAGE_TABLE_FLAG_PRESENT) {
            void * paddr = UINT2PTR(mmu_table_get_addr(user_table, i));
            mmu_table_set(user_table, i, 0, 0);
            ram_page_free(paddr);
        }
    }

    void * page_paddr = UINT2PTR(mmu_table_get_addr(last_table, 1));
    ram_page_free(page_paddr);

    mmu_table_set(last_table, 1, 0, 0);
    mmu_dir_set(dir, 1, 0, 0);
}
