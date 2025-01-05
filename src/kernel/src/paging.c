#include "paging.h"

#include "libc/string.h"
#include "ram.h"

static uint32_t temp_pages[VADDR_TMP_PAGE_COUNT];

void paging_init() {
    kmemset(temp_pages, 0, sizeof(temp_pages));
}

void * paging_temp_map(uint32_t paddr) {
    if (!paddr || paddr & MASK_FLAGS) {
        return 0;
    }

    for (size_t i = 0; i < VADDR_TMP_PAGE_COUNT; i++) {
        if (!temp_pages[i]) {
            temp_pages[i] = paddr;

            size_t table_i = ADDR2PAGE(VADDR_TMP_PAGE) + i;

            mmu_dir_t *   dir   = mmu_get_curr_dir();
            mmu_table_t * table = VIRTUAL_TABLE(table_i);
            mmu_table_set(table, table_i, paddr, MMU_TABLE_RW_USER);
            return UINT2PTR(PAGE2ADDR(table_i));
        }
    }

    return 0;
}

void paging_temp_free(uint32_t paddr) {
    if (!paddr || paddr & MASK_FLAGS) {
        return;
    }

    for (size_t i = 0; i < VADDR_TMP_PAGE_COUNT; i++) {
        if (temp_pages[i] == paddr) {
            temp_pages[i] = 0;

            size_t table_i = ADDR2PAGE(VADDR_TMP_PAGE) + i;

            mmu_dir_t *   dir   = mmu_get_curr_dir();
            mmu_table_t * table = VIRTUAL_TABLE(table_i);
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

int paging_map(uint32_t vaddr, uint32_t paddr, enum MMU_TABLE_FLAG flags) {
    if (vaddr & MASK_FLAGS || paddr & MASK_FLAGS) {
        return -1;
    }

    size_t page_i  = ADDR2PAGE(vaddr);
    size_t dir_i   = page_i / MMU_DIR_SIZE;
    size_t table_i = page_i % MMU_DIR_SIZE;

    mmu_dir_t * dir = mmu_get_curr_dir();

    if (!(mmu_dir_get_flags(dir, dir_i) & MMU_DIR_FLAG_PRESENT)) {
        return -1;
    }

    mmu_table_t * table = VIRTUAL_TABLE(dir_i);

    mmu_table_set(table, table_i, paddr, flags);

    return 0;
}

int paging_id_map_range(size_t start, size_t end) {
    while (start <= end) {
        if (paging_id_map_page(start++)) {
            return -1;
        }
    }

    return 0;
}

int paging_id_map_page(size_t page) {
    size_t dir_i   = page / MMU_DIR_SIZE;
    size_t table_i = page % MMU_DIR_SIZE;

    mmu_dir_t * dir = mmu_get_curr_dir();

    if (!(mmu_dir_get_flags(dir, dir_i) & MMU_DIR_FLAG_PRESENT)) {
        return -1;
    }

    mmu_table_t * table = VIRTUAL_TABLE(dir_i);

    mmu_table_set(table, page, page << 12, MMU_TABLE_RW);

    return 0;
}
