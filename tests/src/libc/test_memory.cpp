#include <cstdlib>
#include <cstring>
#include <string>

#include "test_common.h"

extern "C" {
#include "libc/memory.h"
#include "memory_alloc.h"

FAKE_VALUE_FUNC(void *, _page_alloc, size_t);
}

class LibC : public ::testing::Test {
protected:
    void SetUp() override {
        init_mocks();

        RESET_FAKE(_page_alloc);
    }
};

TEST_F(LibC, kmalloc) {
    void * ptr = kmalloc(12);
    EXPECT_EQ(1, memory_alloc_fake.call_count);
    EXPECT_NE(nullptr, memory_alloc_fake.arg0_val);
    EXPECT_EQ(12, memory_alloc_fake.arg1_val);

    EXPECT_EQ(1, memory_init_fake.call_count);
    EXPECT_NE(nullptr, memory_init_fake.arg0_val);
}

TEST_F(LibC, krealloc) {
    char buff[12];

    void * ptr = krealloc(buff, 12);
    EXPECT_EQ(1, memory_realloc_fake.call_count);
    EXPECT_NE(nullptr, memory_realloc_fake.arg0_val);
    EXPECT_EQ(buff, memory_realloc_fake.arg1_val);
    EXPECT_EQ(12, memory_realloc_fake.arg2_val);

    EXPECT_EQ(1, memory_init_fake.call_count);
    EXPECT_NE(nullptr, memory_init_fake.arg0_val);
}

TEST_F(LibC, kfree) {
    char buff[12];
    kfree(buff);

    EXPECT_EQ(1, memory_free_fake.call_count);
    EXPECT_NE(nullptr, memory_free_fake.arg0_val);
    EXPECT_EQ(buff, memory_free_fake.arg1_val);
}
