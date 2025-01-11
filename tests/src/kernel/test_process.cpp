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

TEST_F(Process, process_from_vars) {
    EXPECT_NE(0, process_from_vars(0, 0, 0, 0));

    EXPECT_EQ(0, process_from_vars(&proc, 1, 0x2000, 3));
    EXPECT_EQ(0, ram_page_alloc_fake.call_count);

    EXPECT_EQ(0, proc.pid);
    EXPECT_EQ(0x2, proc.next_heap_page);
    EXPECT_EQ(0, proc.stack_page_count);
    EXPECT_EQ(3, proc.esp);
    EXPECT_EQ(1, proc.cr3);
    EXPECT_EQ(0x10, proc.ss0);
}

TEST_F(Process, process_create) {
    // Invalid Parameters
    EXPECT_NE(0, process_create(0));

    // Fail ram_page_alloc
    EXPECT_NE(0, process_create(&proc));
    EXPECT_EQ(1, proc.pid);

    uint32_t page_ret_seq[2] = {0x400000, 0};

    SET_RETURN_SEQ(ram_page_alloc, page_ret_seq, 2);

    // paging_temp_map fails
    EXPECT_NE(0, process_create(&proc));
    EXPECT_EQ(2, proc.pid);
    EXPECT_EQ(1, ram_page_free_fake.call_count);

    SetUp();

    SET_RETURN_SEQ(ram_page_alloc, page_ret_seq, 2);
    paging_temp_map_fake.return_val = &dir;

    // Second ram page alloc fails
    EXPECT_NE(0, process_create(&proc));
    EXPECT_EQ(3, proc.pid);
    EXPECT_EQ(1, paging_temp_free_fake.call_count);
    EXPECT_EQ(1, ram_page_free_fake.call_count);
    EXPECT_EQ(paging_temp_map_fake.call_count, paging_temp_free_fake.call_count);

    // TODO process_grow_stack fails

    SetUp();

    ram_page_alloc_fake.return_val  = 0x400000;
    paging_temp_map_fake.return_val = &dir;

    // Success
    EXPECT_EQ(0, process_create(&proc));
    EXPECT_EQ(4, proc.pid);
    EXPECT_EQ(VADDR_USER_MEM, proc.next_heap_page);
    EXPECT_EQ(0x400000, proc.cr3);
    EXPECT_EQ(VADDR_USER_STACK, proc.esp);
    EXPECT_EQ(1, proc.stack_page_count);
    EXPECT_EQ(0x10, proc.ss0);

    EXPECT_EQ(3, paging_temp_free_fake.call_count);
    EXPECT_EQ(0x400000, paging_temp_free_fake.arg0_val);
    EXPECT_EQ(0, ram_page_free_fake.call_count);

    EXPECT_EQ(1, mmu_dir_clear_fake.call_count);
    EXPECT_EQ(&dir, mmu_dir_clear_fake.arg0_val);

    EXPECT_EQ(3, mmu_dir_set_fake.call_count);
    EXPECT_EQ(paging_temp_map_fake.call_count, paging_temp_free_fake.call_count);
}

TEST_F(Process, process_free) {
    // Invalid parameters
    EXPECT_NE(0, process_free(0));

    paging_temp_map_fake.return_val = 0;

    // paging_temp_map fails
    EXPECT_NE(0, process_free(&proc));

    SetUp();

    void * paging_temp_map_seq[2] = {&dir, 0};
    SET_RETURN_SEQ(paging_temp_map, paging_temp_map_seq, 2);

    mmu_dir_get_flags_fake.return_val = MMU_DIR_FLAG_PRESENT;

    // second paging_temp_map fails
    EXPECT_NE(0, process_free(&proc));
    EXPECT_EQ(1, mmu_dir_get_flags_fake.call_count);
    EXPECT_EQ(1, mmu_dir_get_addr_fake.call_count);
    EXPECT_EQ(2, paging_temp_map_fake.call_count);
    EXPECT_EQ(1, paging_temp_free_fake.call_count);

    SetUp();

    // TODO Success

    EXPECT_EQ(paging_temp_map_fake.call_count, paging_temp_free_fake.call_count);
}

TEST_F(Process, process_add_pages) {
    // Invalid Parameters
    EXPECT_EQ(0, process_add_pages(0, 0));
    EXPECT_EQ(0, process_add_pages(0, 1));
    EXPECT_EQ(0, process_add_pages(&proc, 0));

    // ram_page_alloc fails
    EXPECT_EQ(0, process_add_pages(&proc, 1));
    EXPECT_EQ(0, proc.pid);

    ram_page_alloc_fake.return_val = 0x400000;

    // paging_temp_map fails
    EXPECT_EQ(0, process_add_pages(&proc, 1));

    SetUp();

    ram_page_alloc_fake.return_val = 0x400000;
    void * paging_temp_map_seq[2]  = {&dir, 0};
    SET_RETURN_SEQ(paging_temp_map, paging_temp_map_seq, 2);

    // second paging_temp_map fails
    EXPECT_EQ(0, process_add_pages(&proc, 1));
    EXPECT_EQ(1, paging_temp_free_fake.call_count);
    EXPECT_EQ(1, ram_page_free_fake.call_count);

    SetUp();

    // TODO Success

    EXPECT_EQ(paging_temp_map_fake.call_count, paging_temp_free_fake.call_count);
}

TEST_F(Process, process_grow_stack) {
    // Invalid Parameters
    EXPECT_NE(0, process_grow_stack(0));

    // paging_temp_map fails
    EXPECT_NE(0, process_grow_stack(&proc));

    SetUp();

    paging_temp_map_fake.return_val = &dir;

    // mmu_dir_get_flags is not present and ram_page_alloc fails
    EXPECT_NE(0, process_grow_stack(&proc));
    EXPECT_EQ(1, ram_page_alloc_fake.call_count);
    EXPECT_EQ(1, paging_temp_free_fake.call_count);
    EXPECT_EQ(paging_temp_map_fake.call_count, paging_temp_free_fake.call_count);

    SetUp();

    void * paging_temp_map_seq[2] = {&dir, 0};
    SET_RETURN_SEQ(paging_temp_map, paging_temp_map_seq, 2);

    mmu_dir_get_flags_fake.return_val = MMU_DIR_FLAG_PRESENT;

    // second paging_temp_map fails
    EXPECT_NE(0, process_grow_stack(&proc));
    EXPECT_EQ(1, paging_temp_free_fake.call_count);

    SetUp();

    mmu_dir_get_flags_fake.return_val = MMU_DIR_FLAG_PRESENT;
    paging_temp_map_fake.return_val   = &dir;

    // second ram_page_alloc fails
    EXPECT_NE(0, process_grow_stack(&proc));
    EXPECT_EQ(2, paging_temp_free_fake.call_count);
    EXPECT_EQ(paging_temp_map_fake.call_count, paging_temp_free_fake.call_count);

    SetUp();

    ram_page_alloc_fake.return_val  = 0x400000;
    paging_temp_map_fake.return_val = &dir;

    // Success, needs new table
    EXPECT_EQ(0, process_grow_stack(&proc));
    EXPECT_EQ(2, ram_page_alloc_fake.call_count);

    EXPECT_EQ(1, mmu_dir_set_fake.call_count);
    EXPECT_EQ(&dir, mmu_dir_set_fake.arg0_val);
    EXPECT_EQ(MMU_DIR_SIZE - 1, mmu_dir_set_fake.arg1_val);
    EXPECT_EQ(0x400000, mmu_dir_set_fake.arg2_val);
    EXPECT_EQ(MMU_DIR_RW, mmu_dir_set_fake.arg3_val);

    EXPECT_EQ(1, mmu_table_set_fake.call_count);
    EXPECT_EQ((mmu_table_t *)&dir, mmu_table_set_fake.arg0_val);
    EXPECT_EQ(MMU_TABLE_SIZE - 1, mmu_table_set_fake.arg1_val);
    EXPECT_EQ(0x400000, mmu_table_set_fake.arg2_val);
    EXPECT_EQ(MMU_TABLE_RW, mmu_table_set_fake.arg3_val);

    EXPECT_EQ(2, paging_temp_free_fake.call_count);

    EXPECT_EQ(paging_temp_map_fake.call_count, paging_temp_free_fake.call_count);

    // call a second time and check mmu_table_set gets new second arg
    EXPECT_EQ(0, process_grow_stack(&proc));

    EXPECT_EQ(2, mmu_table_set_fake.call_count);
    EXPECT_EQ((mmu_table_t *)&dir, mmu_table_set_fake.arg0_val);
    EXPECT_EQ(MMU_TABLE_SIZE - 2, mmu_table_set_fake.arg1_val);
    EXPECT_EQ(0x400000, mmu_table_set_fake.arg2_val);
    EXPECT_EQ(MMU_TABLE_RW, mmu_table_set_fake.arg3_val);

    SetUp();

    ram_page_alloc_fake.return_val    = 0x400000;
    paging_temp_map_fake.return_val   = &dir;
    mmu_dir_get_flags_fake.return_val = MMU_DIR_FLAG_PRESENT;

    // Success, doesn't need new table
    EXPECT_EQ(0, process_grow_stack(&proc));
    EXPECT_EQ(1, ram_page_alloc_fake.call_count);
    EXPECT_EQ(0, mmu_dir_set_fake.call_count);
    EXPECT_EQ(2, paging_temp_free_fake.call_count);

    EXPECT_EQ(1, mmu_table_set_fake.call_count);
    EXPECT_EQ((mmu_table_t *)&dir, mmu_table_set_fake.arg0_val);
    EXPECT_EQ(MMU_TABLE_SIZE - 1, mmu_table_set_fake.arg1_val);
    EXPECT_EQ(0x400000, mmu_table_set_fake.arg2_val);
    EXPECT_EQ(MMU_TABLE_RW, mmu_table_set_fake.arg3_val);

    EXPECT_EQ(paging_temp_map_fake.call_count, paging_temp_free_fake.call_count);

    SetUp();

    ram_page_alloc_fake.return_val  = 0x400000;
    paging_temp_map_fake.return_val = &dir;

    proc.stack_page_count = MMU_TABLE_SIZE;

    // Success add second table (mmu_dir_set call with diff arg1)
    EXPECT_EQ(0, process_grow_stack(&proc));
    EXPECT_EQ(2, ram_page_alloc_fake.call_count);

    EXPECT_EQ(1, mmu_dir_set_fake.call_count);
    EXPECT_EQ(&dir, mmu_dir_set_fake.arg0_val);
    EXPECT_EQ(MMU_DIR_SIZE - 2, mmu_dir_set_fake.arg1_val);
    EXPECT_EQ(0x400000, mmu_dir_set_fake.arg2_val);
    EXPECT_EQ(MMU_DIR_RW, mmu_dir_set_fake.arg3_val);

    EXPECT_EQ(1, mmu_table_set_fake.call_count);
    EXPECT_EQ((mmu_table_t *)&dir, mmu_table_set_fake.arg0_val);
    EXPECT_EQ(MMU_TABLE_SIZE - 1, mmu_table_set_fake.arg1_val);
    EXPECT_EQ(0x400000, mmu_table_set_fake.arg2_val);
    EXPECT_EQ(MMU_TABLE_RW, mmu_table_set_fake.arg3_val);

    EXPECT_EQ(2, paging_temp_free_fake.call_count);

    EXPECT_EQ(MMU_DIR_SIZE - 2, mmu_dir_get_flags_fake.arg1_val);

    EXPECT_EQ(paging_temp_map_fake.call_count, paging_temp_free_fake.call_count);
}
