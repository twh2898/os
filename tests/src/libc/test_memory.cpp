#include <cstdlib>
#include <string>

#include "test_header.h"

extern "C" {
#include "libc/memory.h"
}

extern "C" {
FAKE_VALUE_FUNC(void *, _malloc, size_t);
FAKE_VALUE_FUNC(void *, _realloc, void *, size_t);
FAKE_VOID_FUNC(_free, void *);
}

class LibC : public ::testing::Test {
protected:
    void SetUp() override {
        RESET_FAKE(_malloc);
        RESET_FAKE(_realloc);
        RESET_FAKE(_free);
    }
};

TEST_F(LibC, kmalloc) {
    _malloc_fake.return_val = (void *)3;
    void * ptr              = kmalloc(12);
    EXPECT_EQ(_malloc_fake.call_count, 1);
    EXPECT_EQ(_malloc_fake.arg0_val, 12);
    EXPECT_EQ(ptr, (void *)3);
}

TEST_F(LibC, kcalloc) {
    _malloc_fake.return_val = (void *)3;
    void * ptr              = kcalloc(12, 0);
    // EXPECT_EQ(_malloc_fake.call_count, 1);
    // EXPECT_EQ(_malloc_fake.arg0_val, 12);
    // EXPECT_EQ(ptr, (void *)3);
}

TEST_F(LibC, krealloc) {
    _realloc_fake.return_val = (void *)3;
    void * ptr               = krealloc((void *)5, 12);
    EXPECT_EQ(_realloc_fake.call_count, 1);
    EXPECT_EQ(_realloc_fake.arg0_val, (void *)5);
    EXPECT_EQ(_realloc_fake.arg1_val, 12);
    EXPECT_EQ(ptr, (void *)3);
}

TEST_F(LibC, kfree) {
    _free((void *)5);
    EXPECT_EQ(_free_fake.call_count, 1);
    EXPECT_EQ(_free_fake.arg0_val, (void *)5);
}
