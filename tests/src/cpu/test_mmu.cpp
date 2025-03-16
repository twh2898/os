#include <cstdlib>
#include <cstring>

#include "test_common.h"

extern "C" {
#include "cpu/mmu.h"

mmu_dir_t   dir;
mmu_table_t table;
}

class MMU : public testing::Test {
protected:
    void SetUp() override {
        init_mocks();

        memset(&dir, 0, sizeof(dir));
        memset(&table, 0, sizeof(table));
    }
};

// mmu_dir_clear

TEST_F(MMU, mmu_dir_clear) {
    dir.entries[0] = 10;

    mmu_dir_clear(&dir);
    EXPECT_EQ(0, dir.entries[0]);
}

// mmu_table_clear

TEST_F(MMU, mmu_table_clear) {
    table.entries[0] = 10;

    mmu_table_clear(&table);
    EXPECT_EQ(0, table.entries[0]);
}

// mmu_dir_set_addr

TEST_F(MMU, mmu_dir_set_addr_InvalidParameters) {
    EXPECT_NE(0, mmu_dir_set_addr(0, 0, 0));
    EXPECT_NE(0, mmu_dir_set_addr(0, MMU_DIR_SIZE, 0));
    EXPECT_NE(0, mmu_dir_set_addr(&dir, MMU_DIR_SIZE, 0));
}

TEST_F(MMU, mmu_dir_set_addr) {
    dir.entries[0] = 0xfff;
    EXPECT_EQ(0, mmu_dir_set_addr(&dir, 0, 0x1234));
    EXPECT_EQ(0x1fff, dir.entries[0]);
}

// mmu_dir_set_flags

TEST_F(MMU, mmu_dir_set_flags_InvalidParameters) {
    EXPECT_NE(0, mmu_dir_set_flags(0, 0, 0));
    EXPECT_NE(0, mmu_dir_set_flags(0, MMU_DIR_SIZE, 0));
    EXPECT_NE(0, mmu_dir_set_flags(&dir, MMU_DIR_SIZE, 0));
}

TEST_F(MMU, mmu_dir_set_flags) {
    dir.entries[0] = 0x1000;
    EXPECT_EQ(0, mmu_dir_set_flags(&dir, 0, MMU_DIR_FLAG_PRESENT | MMU_DIR_FLAG_READ_WRITE));
    EXPECT_EQ(0x1003, dir.entries[0]);
}

// mmu_dir_set

TEST_F(MMU, mmu_dir_set_InvalidParameters) {
    EXPECT_NE(0, mmu_dir_set(0, 0, 0, 0));
    EXPECT_NE(0, mmu_dir_set(0, MMU_DIR_SIZE, 0, 0));
    EXPECT_NE(0, mmu_dir_set(&dir, MMU_DIR_SIZE, 0, 0));
}

TEST_F(MMU, mmu_dir_set) {
    EXPECT_EQ(0, mmu_dir_set(&dir, 0, 0x1000, 0x3));
    EXPECT_EQ(0x1003, dir.entries[0]);
}

// mmu_dir_get_addr

TEST_F(MMU, mmu_dir_get_addr_InvalidParameters) {
    EXPECT_EQ(0, mmu_dir_get_addr(0, 0));
    EXPECT_EQ(0, mmu_dir_get_addr(&dir, MMU_DIR_SIZE));
    EXPECT_EQ(0, mmu_dir_get_addr(0, MMU_DIR_SIZE));
}

TEST_F(MMU, mmu_dir_get_addr) {
    dir.entries[0] = 0x1234;
    EXPECT_EQ(0x1000, mmu_dir_get_addr(&dir, 0));
}

// mmu_dir_get_flags

TEST_F(MMU, mmu_dir_get_flags_InvalidParameters) {
    EXPECT_EQ(0, mmu_dir_get_flags(0, 0));
    EXPECT_EQ(0, mmu_dir_get_flags(&dir, MMU_DIR_SIZE));
    EXPECT_EQ(0, mmu_dir_get_flags(0, MMU_DIR_SIZE));
}

TEST_F(MMU, mmu_dir_get_flags) {
    dir.entries[0] = 0x1234;
    EXPECT_EQ(0x234, mmu_dir_get_flags(&dir, 0));
}

// mmu_table_set_addr

TEST_F(MMU, mmu_table_set_addr_InvalidParameters) {
    EXPECT_NE(0, mmu_table_set_addr(0, 0, 0));
    EXPECT_NE(0, mmu_table_set_addr(0, MMU_TABLE_SIZE, 0));
    EXPECT_NE(0, mmu_table_set_addr(&table, MMU_TABLE_SIZE, 0));
}

TEST_F(MMU, mmu_table_set_addr) {
    table.entries[0] = 0xfff;
    EXPECT_EQ(0, mmu_table_set_addr(&table, 0, 0x1234));
    EXPECT_EQ(0x1fff, table.entries[0]);
}

// mmu_table_set_flags

TEST_F(MMU, mmu_table_set_flags_InvalidParameters) {
    EXPECT_NE(0, mmu_table_set_flags(0, 0, 0));
    EXPECT_NE(0, mmu_table_set_flags(0, MMU_TABLE_SIZE, 0));
    EXPECT_NE(0, mmu_table_set_flags(&table, MMU_TABLE_SIZE, 0));
}

TEST_F(MMU, mmu_table_set_flags) {
    table.entries[0] = 0x1000;
    EXPECT_EQ(0, mmu_table_set_flags(&table, 0, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_READ_WRITE));
    EXPECT_EQ(0x1003, table.entries[0]);
}

// mmu_table_set

TEST_F(MMU, mmu_table_set_InvalidParameters) {
    EXPECT_NE(0, mmu_table_set(0, 0, 0, 0));
    EXPECT_NE(0, mmu_table_set(0, MMU_TABLE_SIZE, 0, 0));
    EXPECT_NE(0, mmu_table_set(&table, MMU_TABLE_SIZE, 0, 0));
}

TEST_F(MMU, mmu_table_set) {
    EXPECT_EQ(0, mmu_table_set(&table, 0, 0x1000, 0x3));
    EXPECT_EQ(0x1003, table.entries[0]);
}

// mmu_table_get_addr

TEST_F(MMU, mmu_table_get_addr_InvalidParameters) {
    EXPECT_EQ(0, mmu_table_get_addr(0, 0));
    EXPECT_EQ(0, mmu_table_get_addr(&table, MMU_TABLE_SIZE));
    EXPECT_EQ(0, mmu_table_get_addr(0, MMU_TABLE_SIZE));
}

TEST_F(MMU, mmu_table_get_addr) {
    table.entries[0] = 0x1234;
    EXPECT_EQ(0x1000, mmu_table_get_addr(&table, 0));
}

// mmu_table_get_flags

TEST_F(MMU, mmu_table_get_flags_InvalidParameters) {
    EXPECT_EQ(0, mmu_table_get_flags(0, 0));
    EXPECT_EQ(0, mmu_table_get_flags(&table, MMU_TABLE_SIZE));
    EXPECT_EQ(0, mmu_table_get_flags(0, MMU_TABLE_SIZE));
}

TEST_F(MMU, mmu_table_get_flags) {
    table.entries[0] = 0x1234;
    EXPECT_EQ(0x234, mmu_table_get_flags(&table, 0));
}
