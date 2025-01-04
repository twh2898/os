#include "exec.h"

#include "cpu/mmu.h"
#include "ram.h"
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
    mmu_page_dir_t *   dir        = UINT2PTR(VADDR_PAGE_DIR);
    mmu_page_table_t * last_table = UINT2PTR(VADDR_LAST_PAGE_TABLE);

    uint32_t page = ram_page_alloc();
    mmu_dir_set(dir, 1, page, MMU_DIR_RW);
    mmu_table_set(last_table, 1, page, MMU_TABLE_RW);

    mmu_page_table_t * user_table = UINT2PTR(mmu_dir_get_vaddr(dir, 1));

    size_t page_count = PAGE_ALIGNED(size) >> 12;
    for (size_t i = 0; i < page_count; i++) {
        uint32_t page_paddr = ram_page_alloc();
        mmu_table_set(user_table, i, page_paddr, MMU_TABLE_RW);
    }
}

static void free_user_space() {
    mmu_page_dir_t *   dir        = UINT2PTR(VADDR_PAGE_DIR);
    mmu_page_table_t * last_table = UINT2PTR(VADDR_LAST_PAGE_TABLE);

    mmu_page_table_t * user_table = UINT2PTR(mmu_dir_get_vaddr(dir, 1));
    mmu_table_clear(user_table);

    mmu_table_set(last_table, 1, 0, 0);
    mmu_dir_set(dir, 1, 0, 0);

    uint32_t page_paddr = mmu_table_get_addr(last_table, 1);
    ram_page_free(page_paddr);
}
