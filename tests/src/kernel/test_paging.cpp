#include <cstdlib>

#include "test_header.h"

#define FIRST_PAGE (VADDR_TMP_PAGE >> 12)

extern "C" {
#include "paging.h"

FAKE_VALUE_FUNC(void *, kmemset, void *, uint8_t, size_t);
FAKE_VALUE_FUNC(mmu_page_dir_t *, mmu_get_curr_dir);
FAKE_VALUE_FUNC(mmu_page_table_t *, mmu_dir_get_table, mmu_page_dir_t *, size_t);
FAKE_VOID_FUNC(mmu_table_set, mmu_page_table_t *, size_t, uint32_t, enum MMU_PAGE_TABLE_FLAG);

void * custom_kmemset(void * ptr, uint8_t val, size_t size) {
    return memset(ptr, val, size);
}
}

void setup_fakes() {
    RESET_FAKE(kmemset);
    RESET_FAKE(mmu_get_curr_dir);
    RESET_FAKE(mmu_dir_get_table);
    RESET_FAKE(mmu_table_set);

    kmemset_fake.custom_fake = custom_kmemset;
}

TEST(PagingStatic, paging_init) {
    setup_fakes();

    paging_init();

    EXPECT_EQ(1, kmemset_fake.call_count);
    EXPECT_NE(nullptr, kmemset_fake.arg0_val);
    EXPECT_EQ(0, kmemset_fake.arg1_val);
    EXPECT_EQ(100, kmemset_fake.arg2_val);

    EXPECT_EQ(25, paging_temp_available());
}

class Paging : public ::testing::Test {
protected:
    void SetUp() override {
        setup_fakes();

        paging_init();
    }
};

TEST_F(Paging, paging_temp_map) {
    mmu_page_dir_t   dir;
    mmu_page_table_t table;

    EXPECT_EQ(nullptr, paging_temp_map(0));

    mmu_get_curr_dir_fake.return_val  = &dir;
    mmu_dir_get_table_fake.return_val = &table;

    EXPECT_NE(nullptr, paging_temp_map(1));
    EXPECT_EQ(24, paging_temp_available());

    EXPECT_EQ(1, mmu_get_curr_dir_fake.call_count);

    EXPECT_EQ(1, mmu_dir_get_table_fake.call_count);
    EXPECT_EQ(&dir, mmu_dir_get_table_fake.arg0_val);
    EXPECT_EQ(FIRST_PAGE, mmu_dir_get_table_fake.arg1_val);

    EXPECT_EQ(1, mmu_table_set_fake.call_count);
    EXPECT_EQ(&table, mmu_table_set_fake.arg0_val);
    EXPECT_EQ(FIRST_PAGE, mmu_table_set_fake.arg1_val);
    EXPECT_EQ(1, mmu_table_set_fake.arg2_val);
    EXPECT_EQ(MMU_TABLE_RW_USER, mmu_table_set_fake.arg3_val);

    for (size_t i = 1; i < 25; i++) {
        EXPECT_NE(nullptr, paging_temp_map(i + 1)) << i;
    }

    EXPECT_EQ(nullptr, paging_temp_map(27));
}

TEST_F(Paging, paging_temp_free) {
    mmu_page_dir_t   dir;
    mmu_page_table_t table;

    paging_temp_map(1);

    setup_fakes();

    mmu_get_curr_dir_fake.return_val  = &dir;
    mmu_dir_get_table_fake.return_val = &table;

    paging_temp_free(2);

    EXPECT_EQ(24, paging_temp_available());

    paging_temp_free(1);

    EXPECT_EQ(25, paging_temp_available());

    EXPECT_EQ(1, mmu_get_curr_dir_fake.call_count);

    EXPECT_EQ(1, mmu_dir_get_table_fake.call_count);
    EXPECT_EQ(&dir, mmu_dir_get_table_fake.arg0_val);
    EXPECT_EQ(FIRST_PAGE, mmu_dir_get_table_fake.arg1_val);

    EXPECT_EQ(1, mmu_table_set_fake.call_count);
    EXPECT_EQ(&table, mmu_table_set_fake.arg0_val);
    EXPECT_EQ(FIRST_PAGE, mmu_table_set_fake.arg1_val);
    EXPECT_EQ(0, mmu_table_set_fake.arg2_val);
    EXPECT_EQ(0, mmu_table_set_fake.arg3_val);
}

TEST_F(Paging, paging_temp_available) {
    EXPECT_EQ(25, paging_temp_available());

    paging_temp_map(1);

    EXPECT_EQ(24, paging_temp_available());

    paging_temp_free(1);

    EXPECT_EQ(25, paging_temp_available());

    for (size_t i = 0; i < 25; i++) {
        EXPECT_NE(nullptr, paging_temp_map(i + 1)) << i;
    }

    EXPECT_EQ(0, paging_temp_available());
}
