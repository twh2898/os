extern "C" {
#include "libk/sys_call.h"
}

#include <gtest/gtest.h>

#include <string>

#include "fff.h"

DEFINE_FFF_GLOBALS;

extern "C" {
// FAKE_VALUE_FUNC(uint32_t, send_interrupt, uint32_t);
FAKE_VALUE_FUNC(uint32_t, send_interrupt, uint32_t, uint32_t, uint32_t);
}

class LibK : public ::testing::Test {
protected:
    void SetUp() override {
        RESET_FAKE(send_interrupt);
    }
};

TEST_F(LibK, malloc) {
    send_interrupt_fake.return_val = 3;
    void * ptr                     = _malloc(12);
    EXPECT_EQ(send_interrupt_fake.call_count, 1);
    EXPECT_EQ(send_interrupt_fake.arg0_val, 0x200);
    EXPECT_EQ(send_interrupt_fake.arg1_val, 12);
    EXPECT_EQ(ptr, (void *)3);
}

TEST_F(LibK, realloc) {
    send_interrupt_fake.return_val = 3;
    void * ptr                     = _realloc((void *)5, 12);
    EXPECT_EQ(send_interrupt_fake.call_count, 1);
    EXPECT_EQ(send_interrupt_fake.arg0_val, 0x201);
    EXPECT_EQ(send_interrupt_fake.arg1_val, 5);
    EXPECT_EQ(send_interrupt_fake.arg2_val, 12);
    EXPECT_EQ(ptr, (void *)3);
}

TEST_F(LibK, free) {
    _free((void *)5);
    EXPECT_EQ(send_interrupt_fake.call_count, 1);
    EXPECT_EQ(send_interrupt_fake.arg0_val, 0x202);
    EXPECT_EQ(send_interrupt_fake.arg1_val, 5);
}

TEST_F(LibK, putc) {
    send_interrupt_fake.return_val = 1;
    size_t olen = _putc('A');
    EXPECT_EQ(send_interrupt_fake.call_count, 1);
    EXPECT_EQ(send_interrupt_fake.arg0_val, 0x1000);
    EXPECT_EQ(send_interrupt_fake.arg1_val, 'A');
    EXPECT_EQ(olen, 1);
}

TEST_F(LibK, puts) {
    send_interrupt_fake.return_val = 3;
    size_t olen = _puts("ABC");
    EXPECT_EQ(send_interrupt_fake.call_count, 1);
    EXPECT_EQ(send_interrupt_fake.arg0_val, 0x1001);
    EXPECT_NE(send_interrupt_fake.arg1_val, 0);
    EXPECT_EQ(olen, 3);
}
