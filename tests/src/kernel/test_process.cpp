#include <cstdlib>

#include "test_header.h"

extern "C" {
#define _Noreturn
#include "cpu/mmu.h"
#include "proc.h"

mmu_dir_t dir;
process_t proc;

FAKE_VALUE_FUNC(void *, kmemset, void *, uint8_t, size_t);

FAKE_VALUE_FUNC(uint32_t, ram_page_alloc);
FAKE_VOID_FUNC(ram_page_free, uint32_t);

FAKE_VALUE_FUNC(void *, paging_temp_map, uint32_t);
FAKE_VOID_FUNC(paging_temp_free, uint32_t);

FAKE_VOID_FUNC(mmu_dir_clear, mmu_dir_t *);

FAKE_VALUE_FUNC(int, mmu_dir_set, mmu_dir_t *, size_t, uint32_t, enum MMU_DIR_FLAG);
FAKE_VALUE_FUNC(enum MMU_DIR_FLAG, mmu_dir_get_flags, mmu_dir_t *, size_t);
FAKE_VALUE_FUNC(uint32_t, mmu_dir_get_addr, mmu_dir_t *, size_t);

FAKE_VALUE_FUNC(int, mmu_table_set, mmu_table_t *, size_t, uint32_t, enum MMU_TABLE_FLAG);
FAKE_VALUE_FUNC(enum MMU_TABLE_FLAG, mmu_table_get_flags, mmu_table_t *, size_t);
FAKE_VALUE_FUNC(uint32_t, mmu_table_get_addr, mmu_table_t *, size_t);

void * custom_kmemset(void * ptr, uint8_t val, size_t size) {
    return memset(ptr, val, size);
}
}

class Process : public ::testing::Test {
protected:
    void SetUp() override {
        RESET_FAKE(kmemset);
        RESET_FAKE(ram_page_alloc);
        RESET_FAKE(ram_page_free);
        RESET_FAKE(paging_temp_map);
        RESET_FAKE(paging_temp_free);
        RESET_FAKE(mmu_dir_clear);
        RESET_FAKE(mmu_dir_set);
        RESET_FAKE(mmu_dir_get_flags);
        RESET_FAKE(mmu_dir_get_addr);
        RESET_FAKE(mmu_table_set);
        RESET_FAKE(mmu_table_get_flags);
        RESET_FAKE(mmu_table_get_addr);

        kmemset_fake.custom_fake = custom_kmemset;

        memset(&dir, 0, sizeof(dir));
        memset(&proc, 0, sizeof(proc));
    }
};

TEST_F(Process, process_create) {
    // Invalid Parameters
    EXPECT_NE(0, process_create(0));

    // Fail ram_page_alloc
    EXPECT_NE(0, process_create(&proc));
    EXPECT_EQ(0, proc.pid);

    uint32_t page_ret_seq[2] = {0x400000, 0};

    SET_RETURN_SEQ(ram_page_alloc, page_ret_seq, 2);

    // paging_temp_map fails
    EXPECT_NE(0, process_create(&proc));
    EXPECT_EQ(1, proc.pid);
    EXPECT_EQ(1, ram_page_free_fake.call_count);

    SetUp();

    SET_RETURN_SEQ(ram_page_alloc, page_ret_seq, 2);
    paging_temp_map_fake.return_val = &dir;

    // Second ram page alloc fails
    EXPECT_NE(0, process_create(&proc));
    EXPECT_EQ(2, proc.pid);
    EXPECT_EQ(1, paging_temp_free_fake.call_count);
    EXPECT_EQ(1, ram_page_free_fake.call_count);
    EXPECT_EQ(paging_temp_map_fake.call_count, paging_temp_free_fake.call_count);

    // TODO process_grow_stack fails

    SetUp();

    ram_page_alloc_fake.return_val  = 0x400000;
    paging_temp_map_fake.return_val = &dir;

    // Success
    EXPECT_EQ(0, process_create(&proc));
    EXPECT_EQ(3, proc.pid);
    EXPECT_EQ(VADDR_USER_MEM, proc.next_heap_page);
    EXPECT_EQ(0x400000, proc.cr3);
    EXPECT_EQ(VADDR_USER_STACK, proc.esp);
    EXPECT_EQ(1, proc.stack_page_count);

    EXPECT_EQ(3, paging_temp_free_fake.call_count);
    EXPECT_EQ(0x400000, paging_temp_free_fake.arg0_val);
    EXPECT_EQ(0, ram_page_free_fake.call_count);

    EXPECT_EQ(1, mmu_dir_clear_fake.call_count);
    EXPECT_EQ(&dir, mmu_dir_clear_fake.arg0_val);

    EXPECT_EQ(3, mmu_dir_set_fake.call_count);
    EXPECT_EQ(paging_temp_map_fake.call_count, paging_temp_free_fake.call_count);
}

TEST_F(Process, process_free) {
}

TEST_F(Process, process_add_pages) {
}

TEST_F(Process, process_grow_stack) {
}
