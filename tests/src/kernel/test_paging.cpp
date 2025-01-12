#include <cstdlib>

#include "test_common.h"

extern "C" {
#include "paging.h"
}

#define EXPECT_BALANCED() EXPECT_EQ(25, paging_temp_available())

void setup_fakes() {
    init_mocks();

    mmu_get_curr_dir_fake.return_val = 0x1000;
}

TEST(PagingStatic, paging_init) {
    setup_fakes();

    paging_init();

    EXPECT_EQ(1, kmemset_fake.call_count);
    EXPECT_NE(nullptr, kmemset_fake.arg0_val);
    EXPECT_EQ(0, kmemset_fake.arg1_val);
    EXPECT_EQ(200, kmemset_fake.arg2_val);

    EXPECT_EQ(25, paging_temp_available());
}

class Paging : public ::testing::Test {
protected:
    void SetUp() override {
        setup_fakes();

        paging_init();
    }

    void fill_temp_pages() {
        size_t i = 1;
        while (paging_temp_available()) {
            paging_temp_map(i++ << 12);
        }
    }

    void clear_temp_pages() {
        paging_init();
    }
};

TEST_F(Paging, paging_temp_map) {
    EXPECT_EQ(nullptr, paging_temp_map(0));
    EXPECT_EQ(nullptr, paging_temp_map(1));

    EXPECT_NE(nullptr, paging_temp_map(0x1000));
    EXPECT_EQ(24, paging_temp_available());

    EXPECT_EQ(0, mmu_get_curr_dir_fake.call_count);

    EXPECT_EQ(1, mmu_table_set_fake.call_count);
    EXPECT_EQ((mmu_table_t *)VADDR_KERNEL_TABLE, mmu_table_set_fake.arg0_val);
    EXPECT_EQ(ADDR2PAGE(VADDR_TMP_PAGE), mmu_table_set_fake.arg1_val);
    EXPECT_EQ(0x1000, mmu_table_set_fake.arg2_val);
    EXPECT_EQ(MMU_TABLE_RW_USER, mmu_table_set_fake.arg3_val);

    for (size_t i = 1; i < 25; i++) {
        EXPECT_NE(nullptr, paging_temp_map((i + 1) << 12)) << i;
    }

    EXPECT_EQ(nullptr, paging_temp_map(0x27000));

    SetUp();

    void * a = paging_temp_map(0x1000);
    EXPECT_NE(nullptr, a);
    EXPECT_EQ(24, paging_temp_available());

    void * b = paging_temp_map(0x1000);
    EXPECT_EQ(a, b);
    EXPECT_EQ(24, paging_temp_available());

    EXPECT_EQ(1, mmu_table_set_fake.call_count);

    paging_temp_free(0x1000);
    EXPECT_EQ(24, paging_temp_available());

    paging_temp_free(0x1000);
    EXPECT_EQ(25, paging_temp_available());
}

TEST_F(Paging, paging_temp_free) {
    paging_temp_map(0x1000);

    setup_fakes();

    paging_temp_free(0x2000);
    EXPECT_EQ(24, paging_temp_available());

    paging_temp_free(0x1000);
    EXPECT_EQ(25, paging_temp_available());
    EXPECT_EQ(0, mmu_table_set_fake.call_count);
}

TEST_F(Paging, paging_temp_available) {
    EXPECT_EQ(25, paging_temp_available());

    paging_temp_map(0x1000);
    EXPECT_EQ(24, paging_temp_available());

    paging_temp_free(0x1000);
    EXPECT_EQ(25, paging_temp_available());

    for (size_t i = 0; i < 25; i++) {
        EXPECT_NE(nullptr, paging_temp_map((i + 1) << 12)) << i;
    }

    EXPECT_EQ(0, paging_temp_available());
}

TEST_F(Paging, paging_id_map_range) {
    EXPECT_NE(0, paging_id_map_range(MMU_TABLE_SIZE, MMU_TABLE_SIZE));

    EXPECT_EQ(0, paging_id_map_range(0, 0));
    EXPECT_EQ(0, paging_id_map_range(1, 2));
}

TEST_F(Paging, paging_id_map_page) {
    // EXPECT_NE(0, paging_id_map_page(3));

    // mmu_dir_get_flags_fake.return_val = (enum MMU_DIR_FLAG)MMU_DIR_RW;

    // EXPECT_EQ(0, paging_id_map_page(3));

    // EXPECT_EQ(2, mmu_get_curr_dir_fake.call_count);

    // EXPECT_EQ(2, mmu_dir_get_flags_fake.call_count);
    // EXPECT_EQ(&dir, mmu_dir_get_flags_fake.arg0_val);
    // EXPECT_EQ(0, mmu_dir_get_flags_fake.arg1_val);

    // EXPECT_EQ(1, mmu_table_set_fake.call_count);
    // EXPECT_EQ((mmu_table_t *)VADDR_KERNEL_TABLE, mmu_table_set_fake.arg0_val);
    // EXPECT_EQ(3, mmu_table_set_fake.arg1_val);
    // EXPECT_EQ(0x3000, mmu_table_set_fake.arg2_val);
    // EXPECT_EQ(MMU_TABLE_RW, mmu_table_set_fake.arg3_val);
}

TEST_F(Paging, paging_add_pages) {
}

TEST_F(Paging, paging_remove_pages) {
}

TEST_F(Paging, paging_add_table) {
    // Invalid parameters
    EXPECT_NE(0, paging_add_table(MMU_DIR_SIZE));

    mmu_get_curr_dir_fake.return_val = 0;

    // paging_temp_map fails
    EXPECT_NE(0, paging_add_table(MMU_DIR_SIZE - 1));
    EXPECT_BALANCED();

    SetUp();

    // ram_page_alloc fails
    EXPECT_NE(0, paging_add_table(1));
    EXPECT_EQ(1, ram_page_alloc_fake.call_count);
    EXPECT_BALANCED();

    SetUp();

    ram_page_alloc_fake.return_val = 0x1000;

    // needs table
    EXPECT_EQ(0, paging_add_table(1));
    EXPECT_EQ(1, ram_page_alloc_fake.call_count);
    EXPECT_EQ(1, mmu_dir_set_fake.call_count);
    EXPECT_BALANCED();

    mmu_dir_get_flags_fake.return_val = MMU_DIR_FLAG_PRESENT;

    // table present
    EXPECT_EQ(0, paging_add_table(1));
    EXPECT_EQ(1, ram_page_alloc_fake.call_count);
    EXPECT_BALANCED();
}

TEST_F(Paging, paging_remove_table) {
    // Invalid parameters
    EXPECT_NE(0, paging_remove_table(MMU_DIR_SIZE));

    mmu_get_curr_dir_fake.return_val = 0;

    // paging_temp_map fails
    EXPECT_NE(0, paging_remove_table(MMU_DIR_SIZE - 1));
    EXPECT_BALANCED();

    SetUp();

    // mmu_dir_get_flags not present
    EXPECT_EQ(0, paging_remove_table(0));
    EXPECT_EQ(0, mmu_dir_get_addr_fake.call_count);
    EXPECT_BALANCED();

    mmu_dir_get_flags_fake.return_val = MMU_DIR_FLAG_PRESENT;

    // mmu_dir_get_flags is present
    EXPECT_EQ(0, paging_remove_table(0));
    EXPECT_EQ(1, mmu_dir_get_addr_fake.call_count);
    EXPECT_EQ(1, mmu_dir_set_fake.call_count);
    EXPECT_EQ(1, ram_page_free_fake.call_count);
    EXPECT_BALANCED();
}
