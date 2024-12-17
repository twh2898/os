#include <cstdlib>

#include "test_header.h"

extern "C" {
#include "libc/string.h"
#include "libc/memory.h"
}

extern "C" {
FAKE_VALUE_FUNC(void *, kmalloc, size_t);
FAKE_VALUE_FUNC(void *, kcalloc, size_t, uint8_t);
FAKE_VALUE_FUNC(void *, krealloc, void *, size_t);
FAKE_VOID_FUNC(kfree, void *);

void * custom_kmalloc(size_t size) {
    return malloc(size);
}

void * custom_kcalloc(size_t size, uint8_t value) {
    return calloc(size, value);
}

void * custom_krealloc(void * ptr, size_t size) {
    return realloc(ptr, size);
}

void custom_kfree(void * ptr) {
    free(ptr);
}
}

class String : public testing::Test {
protected:
    void SetUp() override {
        RESET_FAKE(kmalloc);
        RESET_FAKE(kcalloc);
        RESET_FAKE(krealloc);
        RESET_FAKE(kfree);

        kmalloc_fake.custom_fake  = custom_kmalloc;
        kcalloc_fake.custom_fake  = custom_kcalloc;
        krealloc_fake.custom_fake = custom_krealloc;
        kfree_fake.custom_fake    = custom_kfree;
    }
};

TEST_F(String, test_atoi) {
    EXPECT_EQ(0, katoi(0));
    EXPECT_EQ(0, katoi("0"));
    EXPECT_EQ(0, katoi(""));
    EXPECT_EQ(1, katoi("1"));
    EXPECT_EQ(123, katoi("123"));
    EXPECT_EQ(123, katoi("0123"));
    EXPECT_EQ(-123, katoi("-123"));
    EXPECT_EQ(123, katoi("+123"));
}

TEST_F(String, test_char2int) {
    EXPECT_FALSE(char2int('1', 10, 0));

    int i;

    EXPECT_TRUE(char2int('0', 10, &i));
    EXPECT_EQ(0, i);

    EXPECT_TRUE(char2int('1', 10, &i));
    EXPECT_EQ(1, i);

    EXPECT_TRUE(char2int('9', 10, &i));
    EXPECT_EQ(9, i);

    EXPECT_TRUE(char2int('1', 2, &i));
    EXPECT_EQ(1, i);

    EXPECT_TRUE(char2int('9', 16, &i));
    EXPECT_EQ(9, i);

    EXPECT_TRUE(char2int('a', 16, &i));
    EXPECT_EQ(10, i);

    EXPECT_TRUE(char2int('A', 16, &i));
    EXPECT_EQ(10, i);

    EXPECT_TRUE(char2int('f', 16, &i));
    EXPECT_EQ(15, i);
}

TEST_F(String, test_atoib) {
    EXPECT_EQ(0, katoib(0, 2));
    EXPECT_EQ(2, katoib("10", 2));
    EXPECT_EQ(10, katoib("10", 10));
    EXPECT_EQ(16, katoib("10", 16));
    EXPECT_EQ(10, katoib("a", 16));
    EXPECT_EQ(-10, katoib("-a", 16));
    EXPECT_EQ(10, katoib("0a", 16));
}
