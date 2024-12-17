#include <cstdlib>
#include <cstring>
#include <string>

#include "test_header.h"

extern "C" {
#include "libc/memory.h"

void * kmemset(void * ptr, uint8_t value, size_t n) {
    return memset(ptr, value, n);
}

FAKE_VALUE_FUNC(void *, _malloc, size_t);
FAKE_VALUE_FUNC(void *, _realloc, void *, size_t);
FAKE_VOID_FUNC(_free, void *);

void * custom_malloc(size_t size) {
    return malloc(size);
}

void * custom_realloc(void * ptr, size_t size) {
    return realloc(ptr, size);
}

void custom_free(void * ptr) {
    free(ptr);
}
}

class LibC : public ::testing::Test {
protected:
    void SetUp() override {
        RESET_FAKE(_malloc);
        RESET_FAKE(_realloc);
        RESET_FAKE(_free);

        _malloc_fake.custom_fake  = custom_malloc;
        _realloc_fake.custom_fake = custom_realloc;
        _free_fake.custom_fake    = custom_free;
    }
};

TEST_F(LibC, kmalloc) {
    void * ptr = kmalloc(12);
    EXPECT_EQ(_malloc_fake.call_count, 1);
    EXPECT_EQ(_malloc_fake.arg0_val, 12);
    free(ptr);
}

TEST_F(LibC, kcalloc) {
    void * ptr = kcalloc(12, 0);
    EXPECT_EQ(_malloc_fake.call_count, 1);
    EXPECT_EQ(_malloc_fake.arg0_val, 12);
    free(ptr);
}

TEST_F(LibC, krealloc) {
    void * ptr = malloc(5);
    ptr        = krealloc(ptr, 12);
    EXPECT_EQ(_realloc_fake.call_count, 1);
    EXPECT_EQ(_realloc_fake.arg0_val, ptr);
    EXPECT_EQ(_realloc_fake.arg1_val, 12);
    free(ptr);
}

TEST_F(LibC, kfree) {
    _free_fake.custom_fake = nullptr;
    _free((void *)5);
    EXPECT_EQ(_free_fake.call_count, 1);
    EXPECT_EQ(_free_fake.arg0_val, (void *)5);
}
