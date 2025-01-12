#include <cstdlib>
#include <cstring>
#include <string>

#include "test_common.h"

extern "C" {
#include "libc/process.h"

FAKE_VOID_FUNC(_proc_exit, uint8_t);
FAKE_VOID_FUNC(_proc_abort, uint8_t, const char *);
FAKE_VOID_FUNC(_proc_panic, const char *, const char *, unsigned int);

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
        RESET_FAKE(_proc_exit);
        RESET_FAKE(_proc_abort);
        RESET_FAKE(_proc_panic);
    }
};

TEST_F(LibC, proc_exit) {
    proc_exit(12);
    EXPECT_EQ(_proc_exit_fake.call_count, 1);
    EXPECT_EQ(_proc_exit_fake.arg0_val, 12);
}

TEST_F(LibC, proc_abort) {
    const char * msg = "message";
    proc_abort(12, msg);
    EXPECT_EQ(_proc_abort_fake.call_count, 1);
    EXPECT_EQ(_proc_abort_fake.arg0_val, 12);
    EXPECT_EQ((void *)_proc_abort_fake.arg1_val, msg);
}

TEST_F(LibC, proc_panic) {
    const char * msg  = "message";
    const char * file = "file";
    proc_panic(msg, file, 17);
    EXPECT_EQ(_proc_panic_fake.call_count, 1);
    EXPECT_EQ((void *)_proc_panic_fake.arg0_val, msg);
    EXPECT_EQ((void *)_proc_panic_fake.arg1_val, file);
    EXPECT_EQ(_proc_panic_fake.arg2_val, 17);
}
