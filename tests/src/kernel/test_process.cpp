#include <array>
#include <cstdlib>

#include "test_common.h"

extern "C" {
#define _Noreturn

#include "addr.h"
#include "cpu/mmu.h"
#include "process.h"

mmu_dir_t   dir;
mmu_table_t table;
process_t   proc;
}

std::array<char, PAGE_SIZE * 3>                 temp_page;
std::array<char, PAGE_SIZE * 2 + PAGE_SIZE / 2> heap_data;

class Process : public ::testing::Test {
protected:
    void SetUp() override {
        init_mocks();

        memset(&dir, 0, sizeof(dir));
        memset(&table, 0, sizeof(table));
        memset(&proc, 0, sizeof(proc));

        temp_page.fill(0);

        for (size_t i = 0; i < heap_data.size(); i++) {
            heap_data[i] = i % 0xff;
        }

        proc.next_heap_page = 2;

        paging_temp_map_fake.return_val = temp_page.data();
    }
};

// Process Create

TEST_F(Process, process_create_InvalidParameters) {
    EXPECT_NE(0, process_create(0));
}

TEST_F(Process, process_create_FailPageAlloc) {
    EXPECT_NE(0, process_create(&proc));
    EXPECT_NE(0, proc.pid);
    ASSERT_RAM_ALLOC_BALANCE_OFFSET(1);
}

TEST_F(Process, process_create_TempMapFails) {
    paging_temp_map_fake.return_val = 0;

    uint32_t page_ret_seq[2] = {0x400000, 0};

    SET_RETURN_SEQ(ram_page_alloc, page_ret_seq, 2);

    EXPECT_NE(0, process_create(&proc));
    EXPECT_NE(0, proc.pid);
    EXPECT_EQ(1, ram_page_free_fake.call_count);
    ASSERT_TEMP_MAP_BALANCE_OFFSET(1);
    ASSERT_RAM_ALLOC_BALANCED();
}

TEST_F(Process, process_create_FailSecondPageAlloc) {
    uint32_t page_ret_seq[2] = {0x400000, 0};

    SET_RETURN_SEQ(ram_page_alloc, page_ret_seq, 2);
    paging_temp_map_fake.return_val = &dir;

    EXPECT_NE(0, process_create(&proc));
    EXPECT_NE(0, proc.pid);
    EXPECT_EQ(1, paging_temp_free_fake.call_count);
    EXPECT_EQ(1, ram_page_free_fake.call_count);
    ASSERT_TEMP_MAP_BALANCED();
    ASSERT_RAM_ALLOC_BALANCE_OFFSET(1);
}

TEST_F(Process, process_create_FailAddPages) {
    ram_page_alloc_fake.return_val   = 0x400000;
    paging_temp_map_fake.return_val  = &dir;
    paging_add_pages_fake.return_val = -1;

    EXPECT_NE(0, process_create(&proc));
    EXPECT_NE(0, proc.pid);
    EXPECT_EQ(1, paging_temp_free_fake.call_count);
    EXPECT_EQ(2, ram_page_free_fake.call_count);
    ASSERT_TEMP_MAP_BALANCED();
    ASSERT_RAM_ALLOC_BALANCED();
}

TEST_F(Process, process_create) {
    ram_page_alloc_fake.return_val  = 0x2000;
    paging_temp_map_fake.return_val = &dir;

    EXPECT_EQ(0, process_create(&proc));
    EXPECT_NE(0, proc.pid);
    EXPECT_EQ(1024, proc.next_heap_page);
    EXPECT_EQ(0x2000, proc.cr3);
    EXPECT_EQ(VADDR_USER_STACK, proc.esp);
    EXPECT_EQ(1, proc.stack_page_count);
    EXPECT_EQ(VADDR_ISR_STACK, proc.esp0);
    ASSERT_TEMP_MAP_BALANCED();
    ASSERT_RAM_ALLOC_BALANCE_OFFSET(2);
}

// Process Free

TEST_F(Process, process_free_InvalidParameters) {
    EXPECT_NE(0, process_free(0));
}

TEST_F(Process, process_free_FailTempMap) {
    paging_temp_map_fake.return_val = 0;

    EXPECT_NE(0, process_free(&proc));
    ASSERT_TEMP_MAP_BALANCE_OFFSET(1);
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
    EXPECT_EQ(1, ram_page_free_fake.call_count);
    ASSERT_TEMP_MAP_BALANCE_OFFSET(1);
}

TEST_F(Process, process_free_NoPages) {
    paging_temp_map_fake.return_val   = &dir;
    mmu_dir_get_flags_fake.return_val = MMU_DIR_FLAG_PRESENT;

    EXPECT_EQ(0, process_free(&proc));
    EXPECT_EQ(MMU_DIR_SIZE, ram_page_free_fake.call_count); // tables + dir
    ASSERT_TEMP_MAP_BALANCED();
}

TEST_F(Process, process_free_NoTables) {
    paging_temp_map_fake.return_val = &dir;

    EXPECT_EQ(0, process_free(&proc));
    EXPECT_EQ(1, ram_page_free_fake.call_count);
    ASSERT_TEMP_MAP_BALANCED();
}

TEST_F(Process, process_free) {
    paging_temp_map_fake.return_val     = &dir;
    mmu_dir_get_flags_fake.return_val   = MMU_DIR_FLAG_PRESENT;
    mmu_table_get_flags_fake.return_val = MMU_TABLE_FLAG_PRESENT;

    int page_count  = (MMU_DIR_SIZE - 1) * MMU_TABLE_SIZE;
    int table_count = MMU_DIR_SIZE - 1;
    int dir_count   = 1;

    int expect_free_count = page_count + table_count + dir_count;

    EXPECT_EQ(0, process_free(&proc));
    EXPECT_EQ(expect_free_count, ram_page_free_fake.call_count);
    ASSERT_TEMP_MAP_BALANCED();
}

// Process Add Pages

TEST_F(Process, process_add_pages_InvalidParameters) {
    EXPECT_EQ(0, process_add_pages(0, 0));
    EXPECT_EQ(0, process_add_pages(0, 1));
    EXPECT_EQ(0, process_add_pages(&proc, 0));

    proc.next_heap_page = MMU_DIR_SIZE * MMU_TABLE_SIZE - 1;

    // Count will pass end of last table
    EXPECT_EQ(0, process_add_pages(&proc, 1));
}

TEST_F(Process, process_add_pages_FailTempMap) {
    paging_temp_map_fake.return_val = 0;

    EXPECT_EQ(0, process_add_pages(&proc, 1));
    ASSERT_TEMP_MAP_BALANCE_OFFSET(1);
}

TEST_F(Process, process_add_pages_FailAddPages) {
    paging_temp_map_fake.return_val  = &dir;
    paging_add_pages_fake.return_val = -1;

    EXPECT_EQ(0, process_add_pages(&proc, 1));
    ASSERT_TEMP_MAP_BALANCED();
}

TEST_F(Process, process_add_pages) {
    paging_temp_map_fake.return_val = &dir;

    int next_heap = proc.next_heap_page;

    EXPECT_NE(nullptr, process_add_pages(&proc, 1));
    EXPECT_EQ(1, paging_add_pages_fake.call_count);
    EXPECT_EQ(next_heap, paging_add_pages_fake.arg1_val);
    EXPECT_EQ(next_heap + 1, paging_add_pages_fake.arg2_val);
    EXPECT_EQ(next_heap + 1, proc.next_heap_page);
    ASSERT_TEMP_MAP_BALANCED();
}

// Process Grow Stack

TEST_F(Process, process_grow_stack_InvalidParameters) {
    EXPECT_NE(0, process_grow_stack(0));
}

TEST_F(Process, process_grow_stack_FailTempMap) {
    paging_temp_map_fake.return_val = 0;

    EXPECT_NE(0, process_grow_stack(&proc));
    ASSERT_TEMP_MAP_BALANCE_OFFSET(1);
}

TEST_F(Process, process_grow_stack_FailAddPages) {
    paging_temp_map_fake.return_val  = &dir;
    paging_add_pages_fake.return_val = -1;

    EXPECT_NE(0, process_grow_stack(&proc));
    EXPECT_EQ(1, paging_temp_free_fake.call_count);
    ASSERT_TEMP_MAP_BALANCED();
}

TEST_F(Process, process_grow_stack) {
    paging_temp_map_fake.return_val = &dir;

    EXPECT_EQ(0, process_grow_stack(&proc));
    EXPECT_EQ(1, paging_temp_free_fake.call_count);
    ASSERT_TEMP_MAP_BALANCED();
}

// Process Load Heap

TEST_F(Process, process_load_heap_InvalidParameters) {
    EXPECT_NE(0, process_load_heap(0, 0, 0));
    EXPECT_NE(0, process_load_heap(&proc, 0, 0));
    EXPECT_NE(0, process_load_heap(0, heap_data.data(), 0));
    EXPECT_NE(0, process_load_heap(0, 0, 1));
    EXPECT_NE(0, process_load_heap(&proc, heap_data.data(), 0));
    EXPECT_NE(0, process_load_heap(&proc, 0, 1));
    EXPECT_NE(0, process_load_heap(0, heap_data.data(), 1));
}

TEST_F(Process, process_load_heap_FailAddPages) {
    paging_temp_map_fake.return_val = 0;

    EXPECT_NE(0, process_load_heap(&proc, heap_data.data(), PAGE_SIZE));
    EXPECT_EQ(1, paging_temp_map_fake.call_count);
    ASSERT_TEMP_MAP_BALANCE_OFFSET(1);
}

TEST_F(Process, process_load_heap_FailTempMapDir) {
    void * paging_temp_map_seq[2] = {&dir, 0};
    SET_RETURN_SEQ(paging_temp_map, paging_temp_map_seq, 2);

    EXPECT_NE(0, process_load_heap(&proc, heap_data.data(), heap_data.size()));
    EXPECT_EQ(2, paging_temp_map_fake.call_count);
    ASSERT_TEMP_MAP_BALANCE_OFFSET(1);
}

TEST_F(Process, process_load_heap_FailTempMapTable) {
    void * paging_temp_map_seq[3] = {&dir, &dir, 0};
    SET_RETURN_SEQ(paging_temp_map, paging_temp_map_seq, 3);

    EXPECT_NE(0, process_load_heap(&proc, heap_data.data(), heap_data.size()));
    EXPECT_EQ(3, paging_temp_map_fake.call_count);
    ASSERT_TEMP_MAP_BALANCE_OFFSET(1);
}

TEST_F(Process, process_load_heap_FailTempMapPage) {
    void * paging_temp_map_seq[4] = {&dir, &dir, &table, 0};
    SET_RETURN_SEQ(paging_temp_map, paging_temp_map_seq, 4);

    EXPECT_NE(0, process_load_heap(&proc, heap_data.data(), heap_data.size()));
    EXPECT_EQ(4, paging_temp_map_fake.call_count);
    ASSERT_TEMP_MAP_BALANCE_OFFSET(1);
}

TEST_F(Process, process_load_heap_SinglePage) {
    EXPECT_EQ(0, process_load_heap(&proc, heap_data.data(), PAGE_SIZE));
    EXPECT_EQ(1, kmemcpy_fake.call_count);
    EXPECT_EQ(4, paging_temp_map_fake.call_count);
    ASSERT_TEMP_MAP_BALANCED();
}

TEST_F(Process, process_load_heap_MultiplePages) {
    EXPECT_EQ(0, process_load_heap(&proc, heap_data.data(), heap_data.size()));
    EXPECT_EQ(3, kmemcpy_fake.call_count);
    EXPECT_EQ(8, paging_temp_map_fake.call_count);
    ASSERT_TEMP_MAP_BALANCED();
}

TEST_F(Process, process_load_heap_MultipleCalls) {
    EXPECT_EQ(0, process_load_heap(&proc, heap_data.data(), PAGE_SIZE));
    EXPECT_EQ(0, process_load_heap(&proc, heap_data.data(), PAGE_SIZE + 1));

    EXPECT_EQ(3, kmemcpy_fake.call_count);
    EXPECT_EQ(10, paging_temp_map_fake.call_count);
    ASSERT_TEMP_MAP_BALANCED();
}

// Set Next PID

TEST_F(Process, set_next_pid) {
    set_next_pid(1234);

    process_create(&proc);
    EXPECT_EQ(1234, proc.pid);

    process_create(&proc);
    EXPECT_EQ(1235, proc.pid);
}
