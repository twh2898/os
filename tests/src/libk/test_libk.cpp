extern "C" {
#include "libk/sys_call.h"
}

#include <gtest/gtest.h>

#include <string>

#include "fff.h"

DEFINE_FFF_GLOBALS;

extern "C" {
FAKE_VALUE_FUNC(uint32_t, send_interrupt, uint32_t, uint32_t, uint32_t);
FAKE_VALUE_FUNC(uint32_t, send_interrupt_noret, uint32_t, uint32_t, uint32_t, uint32_t);
}

class LibK : public ::testing::Test {
protected:
    void SetUp() override {
        RESET_FAKE(send_interrupt);
        RESET_FAKE(send_interrupt_noret);
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

TEST_F(LibK, exit) {
    _proc_exit(200);
    EXPECT_EQ(send_interrupt_noret_fake.call_count, 1);
    EXPECT_EQ(send_interrupt_noret_fake.arg0_val, 0x300);
    EXPECT_EQ(send_interrupt_noret_fake.arg1_val, 200);
}

TEST_F(LibK, abort) {
    const char * msg = "message";
    _proc_abort(200, msg);
    EXPECT_EQ(send_interrupt_noret_fake.call_count, 1);
    EXPECT_EQ(send_interrupt_noret_fake.arg0_val, 0x301);
    EXPECT_EQ(send_interrupt_noret_fake.arg1_val, 200);
    EXPECT_EQ((void *)send_interrupt_noret_fake.arg2_val, msg);
}

TEST_F(LibK, panic) {
    const char * msg  = "message";
    const char * file = "file";
    unsigned int line = 37;
    _proc_panic(msg, file, line);
    EXPECT_EQ(send_interrupt_noret_fake.call_count, 1);
    EXPECT_EQ(send_interrupt_noret_fake.arg0_val, 0x302);
    EXPECT_EQ((void *)send_interrupt_noret_fake.arg1_val, msg);
    EXPECT_EQ((void *)send_interrupt_noret_fake.arg2_val, file);
    EXPECT_EQ(send_interrupt_noret_fake.arg3_val, line);
}

TEST_F(LibK, putc) {
    send_interrupt_fake.return_val = 1;
    size_t olen                    = _putc('A');
    EXPECT_EQ(send_interrupt_fake.call_count, 1);
    EXPECT_EQ(send_interrupt_fake.arg0_val, 0x1000);
    EXPECT_EQ(send_interrupt_fake.arg1_val, 'A');
    EXPECT_EQ(olen, 1);
}

TEST_F(LibK, puts) {
    const char * str               = "ABC";
    send_interrupt_fake.return_val = 3;
    size_t olen                    = _puts(str);
    EXPECT_EQ(send_interrupt_fake.call_count, 1);
    EXPECT_EQ(send_interrupt_fake.arg0_val, 0x1001);
    EXPECT_EQ(send_interrupt_fake.arg1_val, (uint32_t)str);
    EXPECT_EQ(olen, 3);
}
