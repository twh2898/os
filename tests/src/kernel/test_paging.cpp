#include <cstdlib>

#include "test_header.h"

#define FIRST_PAGE    (VADDR_TMP_PAGE >> 12)
#define TEMP_TABLE(I) VIRTUAL_TABLE(0x9f + (I))

extern "C" {
#include "paging.h"

mmu_dir_t dir;

FAKE_VALUE_FUNC(void *, kmemset, void *, uint8_t, size_t);
FAKE_VALUE_FUNC(mmu_dir_t *, mmu_get_curr_dir);
FAKE_VALUE_FUNC(enum MMU_DIR_FLAG, mmu_dir_get_flags, mmu_dir_t *, size_t);
FAKE_VOID_FUNC(mmu_table_set, mmu_table_t *, size_t, uint32_t, enum MMU_TABLE_FLAG);

void * custom_kmemset(void * ptr, uint8_t val, size_t size) {
    return memset(ptr, val, size);
}
}

void setup_fakes() {
    RESET_FAKE(kmemset);
    RESET_FAKE(mmu_get_curr_dir);
    RESET_FAKE(mmu_dir_get_flags);
    RESET_FAKE(mmu_table_set);

    kmemset_fake.custom_fake         = custom_kmemset;
    mmu_get_curr_dir_fake.return_val = &dir;
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

        memset(&dir, 0, sizeof(dir));

        paging_init();
    }
};

TEST_F(Paging, paging_temp_map) {
    EXPECT_EQ(nullptr, paging_temp_map(0));
    EXPECT_EQ(nullptr, paging_temp_map(1));

    EXPECT_NE(nullptr, paging_temp_map(0x1000));
    EXPECT_EQ(24, paging_temp_available());

    EXPECT_EQ(1, mmu_get_curr_dir_fake.call_count);

    EXPECT_EQ(1, mmu_table_set_fake.call_count);
    EXPECT_EQ(TEMP_TABLE(0), mmu_table_set_fake.arg0_val);
    EXPECT_EQ(FIRST_PAGE, mmu_table_set_fake.arg1_val);
    EXPECT_EQ(0x1000, mmu_table_set_fake.arg2_val);
    EXPECT_EQ(MMU_TABLE_RW_USER, mmu_table_set_fake.arg3_val);

    for (size_t i = 1; i < 25; i++) {
        EXPECT_NE(nullptr, paging_temp_map((i + 1) << 12)) << i;
    }

    EXPECT_EQ(nullptr, paging_temp_map(0x27000));
}

TEST_F(Paging, paging_temp_free) {
    paging_temp_map(0x1000);

    setup_fakes();

    mmu_get_curr_dir_fake.return_val = &dir;

    paging_temp_free(0x2000);

    EXPECT_EQ(24, paging_temp_available());

    paging_temp_free(0x1000);

    EXPECT_EQ(25, paging_temp_available());

    EXPECT_EQ(1, mmu_get_curr_dir_fake.call_count);

    EXPECT_EQ(1, mmu_table_set_fake.call_count);
    EXPECT_EQ(TEMP_TABLE(0), mmu_table_set_fake.arg0_val);
    EXPECT_EQ(FIRST_PAGE, mmu_table_set_fake.arg1_val);
    EXPECT_EQ(0, mmu_table_set_fake.arg2_val);
    EXPECT_EQ(0, mmu_table_set_fake.arg3_val);
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

TEST_F(Paging, paging_map) {
    EXPECT_NE(0, paging_map(0x1000, 0x2000, (enum MMU_TABLE_FLAG)MMU_TABLE_RW));

    EXPECT_EQ(1, mmu_get_curr_dir_fake.call_count);
    EXPECT_EQ(1, mmu_dir_get_flags_fake.call_count);

    mmu_dir_get_flags_fake.return_val = (enum MMU_DIR_FLAG)MMU_DIR_RW;

    // Address not page aligned
    EXPECT_NE(0, paging_map(0x1001, 0x2000, (enum MMU_TABLE_FLAG)MMU_TABLE_RW));
    EXPECT_NE(0, paging_map(0x1000, 0x2001, (enum MMU_TABLE_FLAG)MMU_TABLE_RW));

    // Good
    EXPECT_EQ(0, paging_map(0x1000, 0x2000, (enum MMU_TABLE_FLAG)MMU_TABLE_RW));

    EXPECT_EQ(2, mmu_get_curr_dir_fake.call_count);

    EXPECT_EQ(2, mmu_dir_get_flags_fake.call_count);
    EXPECT_EQ(&dir, mmu_dir_get_flags_fake.arg0_val);
    EXPECT_EQ(0, mmu_dir_get_flags_fake.arg1_val);

    EXPECT_EQ(1, mmu_table_set_fake.call_count);
    EXPECT_EQ(VIRTUAL_TABLE(0), mmu_table_set_fake.arg0_val);
    EXPECT_EQ(1, mmu_table_set_fake.arg1_val);
    EXPECT_EQ(0x2000, mmu_table_set_fake.arg2_val);
    EXPECT_EQ(MMU_TABLE_RW, mmu_table_set_fake.arg3_val);
}

TEST_F(Paging, paging_id_map_range) {
    EXPECT_NE(0, paging_id_map_range(1, 2));

    mmu_dir_get_flags_fake.return_val = (enum MMU_DIR_FLAG)MMU_DIR_RW;

    EXPECT_EQ(0, paging_id_map_range(1, 2));

    EXPECT_EQ(2, mmu_table_set_fake.call_count);

    EXPECT_EQ(VIRTUAL_TABLE(0), mmu_table_set_fake.arg0_history[0]);
    EXPECT_EQ(1, mmu_table_set_fake.arg1_history[0]);
    EXPECT_EQ(0x1000, mmu_table_set_fake.arg2_history[0]);
    EXPECT_EQ(MMU_TABLE_RW, mmu_table_set_fake.arg3_history[0]);

    EXPECT_EQ(VIRTUAL_TABLE(0), mmu_table_set_fake.arg0_history[1]);
    EXPECT_EQ(2, mmu_table_set_fake.arg1_history[1]);
    EXPECT_EQ(0x2000, mmu_table_set_fake.arg2_history[1]);
    EXPECT_EQ(MMU_TABLE_RW, mmu_table_set_fake.arg3_history[1]);
}

TEST_F(Paging, paging_id_map_page) {
    EXPECT_NE(0, paging_id_map_page(3));

    mmu_dir_get_flags_fake.return_val = (enum MMU_DIR_FLAG)MMU_DIR_RW;

    EXPECT_EQ(0, paging_id_map_page(3));

    EXPECT_EQ(2, mmu_get_curr_dir_fake.call_count);

    EXPECT_EQ(2, mmu_dir_get_flags_fake.call_count);
    EXPECT_EQ(&dir, mmu_dir_get_flags_fake.arg0_val);
    EXPECT_EQ(0, mmu_dir_get_flags_fake.arg1_val);

    EXPECT_EQ(1, mmu_table_set_fake.call_count);
    EXPECT_EQ(VIRTUAL_TABLE(0), mmu_table_set_fake.arg0_val);
    EXPECT_EQ(3, mmu_table_set_fake.arg1_val);
    EXPECT_EQ(0x3000, mmu_table_set_fake.arg2_val);
    EXPECT_EQ(MMU_TABLE_RW, mmu_table_set_fake.arg3_val);
}
