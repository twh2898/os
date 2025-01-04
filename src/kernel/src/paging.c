#include "paging.h"

#include "libc/string.h"
#include "ram.h"

static uint32_t temp_pages[VADDR_TMP_PAGE_COUNT];

void paging_init() {
    kmemset(temp_pages, 0, sizeof(temp_pages));
}

void * paging_temp_map(uint32_t paddr) {
    if (!paddr) {
        return 0;
    }

    for (size_t i = 0; i < VADDR_TMP_PAGE_COUNT; i++) {
        if (!temp_pages[i]) {
            temp_pages[i] = paddr;

            size_t table_i = ADDR2PAGE(VADDR_TMP_PAGE) + i;

            mmu_page_dir_t *   dir   = mmu_get_curr_dir();
            mmu_page_table_t * table = mmu_dir_get_table(dir, table_i);
            mmu_table_set(table, table_i, paddr, MMU_TABLE_RW_USER);
            return UINT2PTR(PAGE2ADDR(table_i));
        }
    }

    return 0;
}

void paging_temp_free(uint32_t paddr) {
    if (!paddr) {
        return;
    }

    for (size_t i = 0; i < VADDR_TMP_PAGE_COUNT; i++) {
        if (temp_pages[i] == paddr) {
            temp_pages[i] = 0;

            size_t table_i = ADDR2PAGE(VADDR_TMP_PAGE) + i;

            mmu_page_dir_t *   dir   = mmu_get_curr_dir();
            mmu_page_table_t * table = mmu_dir_get_table(dir, table_i);
            mmu_table_set(table, table_i, 0, 0);

            break;
        }
    }
}

size_t paging_temp_available() {
    size_t free = 0;

    for (size_t i = 0; i < VADDR_TMP_PAGE_COUNT; i++) {
        if (!temp_pages[i]) {
            free++;
        }
    }

    return free;
}
