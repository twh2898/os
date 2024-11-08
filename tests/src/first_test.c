#include <stdbool.h>

#include <memory.h>

#include "libc/memory.c"
#include "test.h"

void * ram_page_alloc() {
    return 0;
}

void kernel_panic(const char * msg, const char * file, unsigned int line) {
    FATAL();
    exit(1);
}

mmu_page_table_t * mmu_dir_get_table(mmu_page_dir_t * dir, size_t i) {
    return 0;
}

void mmu_table_set_addr(mmu_page_table_t * table, size_t i, uint32_t page_addr) {}

void mmu_table_set_flags(mmu_page_table_t * table, size_t i, enum MMU_PAGE_TABLE_FLAG flags) {}

void * kmemset(void * ptr, uint8_t value, size_t n) {
    if (!ptr)
        return 0;
    uint8_t * buf = (uint8_t *)ptr;
    for (size_t i = 0; i < n; i++) {
        *buf++ = value;
    }
    return ptr;
}

int test_init() {
    
    return 0;
}

int test_First() {
    int a = 1;
    int b = a + 2;
    ASSERT_EQ(a + 2, b);
    ASSERT_NOT_EQ(a + 1, b);
    ASSERT_NOT_EQ(a, b);
    ASSERT_TRUE(a > 0);
    ASSERT_FALSE(a > b);
    // FATAL();
    return 0;
}

BEGIN_TESTS
TEST(test_init);
END_TESTS
