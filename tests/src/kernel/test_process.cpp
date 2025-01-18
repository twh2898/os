#include <cstdlib>

#include "test_common.h"

extern "C" {
#define _Noreturn
#include "addr.h"
#include "cpu/mmu.h"
#include "process.h"

mmu_dir_t dir;
process_t proc;
}

class Process : public ::testing::Test {
protected:
    void SetUp() override {
        init_mocks();

        memset(&dir, 0, sizeof(dir));
        memset(&proc, 0, sizeof(proc));

        proc.next_heap_page = 2;
    }

    void expect_temp_balanced() {
        ASSERT_EQ(paging_temp_map_fake.call_count, paging_temp_free_fake.call_count);
    }

    /**
     * @brief  Used when a pageing_temp_map fails (where there is nothing to
     * free for the last call).
     */
    void expect_temp_balance_with_offset(int offset) {
        ASSERT_EQ(paging_temp_map_fake.call_count, paging_temp_free_fake.call_count + offset);
    }
};

#define ASSERT_BALANCED()             ASSERT_NO_FATAL_FAILURE(expect_temp_balanced())
#define ASSERT_BALANCE_OFFSET(OFFSET) ASSERT_NO_FATAL_FAILURE(expect_temp_balance_with_offset(OFFSET))

TEST_F(Process, process_create_InvalidParameters) {
    EXPECT_NE(0, process_create(0));
    ASSERT_BALANCED();
}

TEST_F(Process, process_create_FailPageAlloc) {
    EXPECT_NE(0, process_create(&proc));
    EXPECT_NE(0, proc.pid);
    ASSERT_BALANCED();
}

TEST_F(Process, process_create_TempMapFails) {
    uint32_t page_ret_seq[2] = {0x400000, 0};

    SET_RETURN_SEQ(ram_page_alloc, page_ret_seq, 2);

    EXPECT_NE(0, process_create(&proc));
    EXPECT_NE(0, proc.pid);
    EXPECT_EQ(1, ram_page_free_fake.call_count);
    ASSERT_BALANCE_OFFSET(1);
}

TEST_F(Process, process_create_FailSecondPageAlloc) {
    uint32_t page_ret_seq[2] = {0x400000, 0};

    SET_RETURN_SEQ(ram_page_alloc, page_ret_seq, 2);
    paging_temp_map_fake.return_val = &dir;

    EXPECT_NE(0, process_create(&proc));
    EXPECT_NE(0, proc.pid);
    EXPECT_EQ(1, paging_temp_free_fake.call_count);
    EXPECT_EQ(1, ram_page_free_fake.call_count);
    ASSERT_BALANCED();
}

TEST_F(Process, process_create_FailAddPages) {
    ram_page_alloc_fake.return_val   = 0x400000;
    paging_temp_map_fake.return_val  = &dir;
    paging_add_pages_fake.return_val = -1;

    EXPECT_NE(0, process_create(&proc));
    EXPECT_NE(0, proc.pid);
    EXPECT_EQ(1, paging_temp_free_fake.call_count);
    EXPECT_EQ(1, ram_page_free_fake.call_count);
    ASSERT_BALANCED();
}

TEST_F(Process, process_create_FailGrowStack) {
    void * temp_map_seq[2] = {&dir, 0};

    SET_RETURN_SEQ(paging_temp_map, temp_map_seq, 2);

    ram_page_alloc_fake.return_val  = 0x400000;
    paging_temp_map_fake.return_val = &dir;

    EXPECT_NE(0, process_create(&proc));
    EXPECT_NE(0, proc.pid);
    EXPECT_EQ(1, paging_temp_free_fake.call_count);
    EXPECT_EQ(1, ram_page_free_fake.call_count);
    ASSERT_BALANCE_OFFSET(1);
}

TEST_F(Process, process_create) {
    ram_page_alloc_fake.return_val  = 0x400000;
    paging_temp_map_fake.return_val = &dir;

    // Success
    EXPECT_EQ(0, process_create(&proc));
    EXPECT_NE(0, proc.pid);
    EXPECT_EQ(VADDR_USER_MEM, proc.next_heap_page);
    EXPECT_EQ(0x400000, proc.cr3);
    EXPECT_EQ(VADDR_USER_STACK, proc.esp);
    EXPECT_EQ(1, proc.stack_page_count);
    EXPECT_EQ(VADDR_ISR_STACK, proc.esp0);

    EXPECT_EQ(3, paging_temp_free_fake.call_count);
    EXPECT_EQ(0x400000, paging_temp_free_fake.arg0_val);
    EXPECT_EQ(0, ram_page_free_fake.call_count);

    EXPECT_EQ(1, mmu_dir_clear_fake.call_count);
    EXPECT_EQ(&dir, mmu_dir_clear_fake.arg0_val);

    EXPECT_EQ(3, mmu_dir_set_fake.call_count);
    ASSERT_BALANCED();
}

TEST_F(Process, process_free_InvalidParameters) {
    EXPECT_NE(0, process_free(0));
    ASSERT_BALANCED();
}

TEST_F(Process, process_free_FailTempMap) {
    paging_temp_map_fake.return_val = 0;

    EXPECT_NE(0, process_free(&proc));
    ASSERT_BALANCE_OFFSET(1);
}

TEST_F(Process, process_free_FailSecondTempMap) {
    void * paging_temp_map_seq[2] = {&dir, 0};
    SET_RETURN_SEQ(paging_temp_map, paging_temp_map_seq, 2);

    mmu_dir_get_flags_fake.return_val = MMU_DIR_FLAG_PRESENT;

    EXPECT_NE(0, process_free(&proc));
    EXPECT_EQ(1, mmu_dir_get_flags_fake.call_count);
    EXPECT_EQ(1, mmu_dir_get_addr_fake.call_count);
    EXPECT_EQ(2, paging_temp_map_fake.call_count);
    EXPECT_EQ(1, paging_temp_free_fake.call_count);
    ASSERT_BALANCE_OFFSET(1);
}

TEST_F(Process, process_free_NoPages) {
    paging_temp_map_fake.return_val   = &dir;
    mmu_dir_get_flags_fake.return_val = MMU_DIR_FLAG_PRESENT;

    EXPECT_EQ(0, process_free(&proc));
    ASSERT_BALANCED();
}

TEST_F(Process, process_free_NoTables) {
    paging_temp_map_fake.return_val = &dir;

    EXPECT_EQ(0, process_free(&proc));
    ASSERT_BALANCED();
}

TEST_F(Process, process_free) {
    paging_temp_map_fake.return_val     = &dir;
    mmu_dir_get_flags_fake.return_val   = MMU_DIR_FLAG_PRESENT;
    mmu_table_get_flags_fake.return_val = MMU_TABLE_FLAG_PRESENT;

    EXPECT_EQ(0, process_free(&proc));
    ASSERT_BALANCED();
}

TEST_F(Process, process_add_pages_InvalidParameters) {
    EXPECT_EQ(0, process_add_pages(0, 0));
    EXPECT_EQ(0, process_add_pages(0, 1));
    EXPECT_EQ(0, process_add_pages(&proc, 0));
    ASSERT_BALANCED();
}

TEST_F(Process, process_add_pages_FailPageAlloc) {
    ASSERT_EQ(0, ram_page_alloc_fake.return_val);
    EXPECT_EQ(0, process_add_pages(&proc, 1));
    EXPECT_EQ(0, proc.pid);
    ASSERT_BALANCED();
}

TEST_F(Process, process_add_pages_FailTempMap) {
    ram_page_alloc_fake.return_val = 0x400000;

    EXPECT_EQ(0, process_add_pages(&proc, 1));
    ASSERT_BALANCE_OFFSET(1);
}

TEST_F(Process, process_add_pages_FailSecondTempMap) {
    ram_page_alloc_fake.return_val = 0x400000;
    void * paging_temp_map_seq[2]  = {&dir, 0};
    SET_RETURN_SEQ(paging_temp_map, paging_temp_map_seq, 2);

    EXPECT_EQ(0, process_add_pages(&proc, 1));
    EXPECT_EQ(1, paging_temp_free_fake.call_count);
    EXPECT_EQ(1, ram_page_free_fake.call_count);
    ASSERT_BALANCE_OFFSET(1);
}

TEST_F(Process, process_add_pages) {
    ram_page_alloc_fake.return_val  = 0x400000;
    paging_temp_map_fake.return_val = &dir;

    EXPECT_NE(nullptr, process_add_pages(&proc, 1));
    ASSERT_BALANCED();
}

TEST_F(Process, process_grow_stack_InvalidParameters) {
    EXPECT_NE(0, process_grow_stack(0));
    ASSERT_BALANCED();
}

TEST_F(Process, process_grow_stack_FailTempMap) {
    EXPECT_NE(0, process_grow_stack(&proc));
    ASSERT_BALANCE_OFFSET(1);
}

TEST_F(Process, process_grow_stack_DirNotPresent_FailPageAlloc) {
    paging_temp_map_fake.return_val = &dir;

    // mmu_dir_get_flags is not present and ram_page_alloc fails
    EXPECT_NE(0, process_grow_stack(&proc));
    EXPECT_EQ(1, ram_page_alloc_fake.call_count);
    EXPECT_EQ(1, paging_temp_free_fake.call_count);
    ASSERT_BALANCED();
}

TEST_F(Process, process_grow_stack_FailSecondTempMap) {
    void * paging_temp_map_seq[2] = {&dir, 0};
    SET_RETURN_SEQ(paging_temp_map, paging_temp_map_seq, 2);

    mmu_dir_get_flags_fake.return_val = MMU_DIR_FLAG_PRESENT;

    EXPECT_NE(0, process_grow_stack(&proc));
    EXPECT_EQ(1, paging_temp_free_fake.call_count);
    ASSERT_BALANCE_OFFSET(1);
}

TEST_F(Process, process_grow_stack_FailSecondPageAlloc) {
    mmu_dir_get_flags_fake.return_val = MMU_DIR_FLAG_PRESENT;
    paging_temp_map_fake.return_val   = &dir;

    EXPECT_NE(0, process_grow_stack(&proc));
    EXPECT_EQ(2, paging_temp_free_fake.call_count);
    ASSERT_BALANCED();
}

TEST_F(Process, process_grow_stack_Success_NeedTabble) {
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

    ASSERT_BALANCED();

    // call a second time and check mmu_table_set gets new second arg
    EXPECT_EQ(0, process_grow_stack(&proc));

    EXPECT_EQ(2, mmu_table_set_fake.call_count);
    EXPECT_EQ((mmu_table_t *)&dir, mmu_table_set_fake.arg0_val);
    EXPECT_EQ(MMU_TABLE_SIZE - 2, mmu_table_set_fake.arg1_val);
    EXPECT_EQ(0x400000, mmu_table_set_fake.arg2_val);
    EXPECT_EQ(MMU_TABLE_RW, mmu_table_set_fake.arg3_val);
    ASSERT_BALANCED();
}

TEST_F(Process, process_grow_stack_Success) {
    ram_page_alloc_fake.return_val    = 0x400000;
    paging_temp_map_fake.return_val   = &dir;
    mmu_dir_get_flags_fake.return_val = MMU_DIR_FLAG_PRESENT;

    EXPECT_EQ(0, process_grow_stack(&proc));
    EXPECT_EQ(1, ram_page_alloc_fake.call_count);
    EXPECT_EQ(0, mmu_dir_set_fake.call_count);
    EXPECT_EQ(2, paging_temp_free_fake.call_count);

    EXPECT_EQ(1, mmu_table_set_fake.call_count);
    EXPECT_EQ((mmu_table_t *)&dir, mmu_table_set_fake.arg0_val);
    EXPECT_EQ(MMU_TABLE_SIZE - 1, mmu_table_set_fake.arg1_val);
    EXPECT_EQ(0x400000, mmu_table_set_fake.arg2_val);
    EXPECT_EQ(MMU_TABLE_RW, mmu_table_set_fake.arg3_val);
    ASSERT_BALANCED();
}

TEST_F(Process, process_grow_stack_Success_AddSecondTable) {
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
    ASSERT_BALANCED();
}
