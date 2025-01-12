#include "paging.h"

#include "libc/string.h"
#include "ram.h"

typedef struct {
    uint32_t addr;
    size_t   count;
} page_user_t;

static page_user_t temp_pages[VADDR_TMP_PAGE_COUNT];

void paging_init() {
    kmemset(temp_pages, 0, sizeof(temp_pages));
}

void * paging_temp_map(uint32_t paddr) {
    if (!paddr || paddr & MASK_FLAGS) {
        return 0;
    }

    // Return one if already exists
    for (size_t i = 0; i < VADDR_TMP_PAGE_COUNT; i++) {
        if (temp_pages[i].addr == paddr) {
            temp_pages[i].count++;
            size_t table_i = ADDR2PAGE(VADDR_TMP_PAGE) + i;
            return UINT2PTR(PAGE2ADDR(table_i));
        }
    }

    // Find a free temp page to use
    for (size_t i = 0; i < VADDR_TMP_PAGE_COUNT; i++) {
        if (temp_pages[i].count < 1) {
            temp_pages[i].addr  = paddr;
            temp_pages[i].count = 1;

            size_t table_i = ADDR2PAGE(VADDR_TMP_PAGE) + i;

            mmu_table_t * table = (mmu_table_t *)VADDR_KERNEL_TABLE;
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
        if (temp_pages[i].addr == paddr) {
            if (temp_pages[i].count < 1) {
                return;
            }

            temp_pages[i].count--;

            break;
        }
    }
}

size_t paging_temp_available() {
    size_t free = 0;

    for (size_t i = 0; i < VADDR_TMP_PAGE_COUNT; i++) {
        if (!temp_pages[i].count) {
            free++;
        }
    }

    return free;
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
    if (page >= MMU_TABLE_SIZE) {
        return -1;
    }

    mmu_table_t * table = (mmu_table_t *)VADDR_KERNEL_TABLE;
    mmu_table_set(table, page, page << 12, MMU_TABLE_RW);

    return 0;
}

int paging_add_pages(size_t start, size_t end) {
    if (start > end) {
        return -1;
    }

    // Assuming mmu_get_curr_dir will always be valid
    uint32_t    dir_addr = mmu_get_curr_dir();
    mmu_dir_t * dir      = paging_temp_map(dir_addr);

    if (!dir) {
        return -1;
    }

    uint32_t table_start = start / MMU_DIR_SIZE;
    uint32_t table_end   = end / MMU_DIR_SIZE;

    // Add page tables if missing
    for (size_t dir_i = start; dir_i <= end; dir_i++) {
        if (mmu_dir_get_flags(dir, dir_i) & MMU_DIR_FLAG_PRESENT) {
            continue;
        }

        uint32_t addr = ram_page_alloc();

        if (!addr) {
            paging_temp_free(dir_addr);
            return -1;
        }

        mmu_dir_set(dir, dir_i, addr, MMU_DIR_RW);
    }

    // Add pages to tables
    for (size_t page_i = start; page_i <= end; page_i++) {
        uint32_t addr = ram_page_alloc();

        if (!addr) {
            paging_remove_pages(start, page_i - 1);
            paging_temp_free(dir_addr);
            return -1;
        }

        uint32_t dir_i   = page_i / MMU_DIR_SIZE;
        uint32_t table_i = page_i % MMU_TABLE_SIZE;

        // Table will be present after previous step
        uint32_t      table_addr = mmu_dir_get_addr(dir, dir_i);
        mmu_table_t * table      = paging_temp_map(table_addr);

        if (!table) {
            paging_remove_pages(start, page_i - 1);
            paging_temp_free(table_addr);
            paging_temp_free(dir_addr);
            ram_page_free(addr);
            return -1;
        }

        mmu_table_set(table, table_i, addr, MMU_TABLE_RW);
        paging_temp_free(table_addr);
    }

    paging_temp_free(dir_addr);

    return 0;
}

int paging_remove_pages(size_t start, size_t end) {
    if (start > end) {
        return -1;
    }

    // Assuming mmu_get_curr_dir will always be valid
    uint32_t    dir_addr = mmu_get_curr_dir();
    mmu_dir_t * dir      = paging_temp_map(dir_addr);

    if (!dir) {
        return -1;
    }

    // Remove pages from tables
    for (size_t page_i = start; page_i <= end; page_i++) {
        uint32_t dir_i   = page_i / MMU_DIR_SIZE;
        uint32_t table_i = page_i % MMU_TABLE_SIZE;

        // Table is not present
        if (!(mmu_dir_get_flags(dir, dir_i) & MMU_DIR_FLAG_PRESENT)) {
            continue;
        }

        // Table will be present after previous step
        uint32_t      table_addr = mmu_dir_get_addr(dir, dir_i);
        mmu_table_t * table      = paging_temp_map(table_addr);

        if (!table) {
            paging_temp_free(dir_addr);
            return -1;
        }

        if (!(mmu_table_get_flags(table, table_i) & MMU_TABLE_FLAG_PRESENT)) {
            paging_temp_free(table_addr);
            continue;
        }

        uint32_t page_addr = mmu_table_get_addr(table, table_i);

        mmu_table_set(table, table_i, 0, 0);
        ram_page_free(page_addr);
        paging_temp_free(table_addr);
    }

    paging_temp_free(dir_addr);

    return 0;
}

int paging_add_table(size_t dir_i) {
    if (dir_i > MMU_DIR_SIZE) {
        return -1;
    }

    // Assuming mmu_get_curr_dir will always be valid
    uint32_t    dir_addr = mmu_get_curr_dir();
    mmu_dir_t * dir      = paging_temp_map(dir_addr);

    if (!dir) {
        return -1;
    }

    if (!(mmu_dir_get_flags(dir, dir_i) & MMU_DIR_FLAG_PRESENT)) {
        uint32_t addr = ram_page_alloc();

        if (!addr) {
            paging_temp_free(dir_addr);
            return -1;
        }

        mmu_dir_set(dir, dir_i, addr, MMU_DIR_RW);
    }

    paging_temp_free(dir_addr);

    return 0;
}

int paging_remove_table(size_t dir_i) {
    if (dir_i > MMU_DIR_SIZE) {
        return -1;
    }

    // Assuming mmu_get_curr_dir will always be valid
    uint32_t    dir_addr = mmu_get_curr_dir();
    mmu_dir_t * dir      = paging_temp_map(dir_addr);

    if (!dir) {
        return -1;
    }

    if (mmu_dir_get_flags(dir, dir_i) & MMU_DIR_FLAG_PRESENT) {
        uint32_t addr = mmu_dir_get_addr(dir, dir_i);
        mmu_dir_set(dir, dir_i, 0, 0);
        ram_page_free(addr);
    }

    paging_temp_free(dir_addr);

    return 0;
}
