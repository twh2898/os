#include <cstdlib>
#include <cstring>
#include <string>

#include "test_common.h"

extern "C" {
#include "libc/proc.h"
}

class LibC : public ::testing::Test {
protected:
    void SetUp() override {
        init_mocks();
    }
};

TEST_F(LibC, proc_exit) {
    proc_exit(12);
    EXPECT_EQ(_sys_proc_exit_fake.call_count, 1);
    EXPECT_EQ(_sys_proc_exit_fake.arg0_val, 12);
}

TEST_F(LibC, proc_abort) {
    const char * msg = "message";
    proc_abort(12, msg);
    EXPECT_EQ(_sys_proc_abort_fake.call_count, 1);
    EXPECT_EQ(_sys_proc_abort_fake.arg0_val, 12);
    EXPECT_EQ((void *)_sys_proc_abort_fake.arg1_val, msg);
}

TEST_F(LibC, proc_panic) {
    const char * msg  = "message";
    const char * file = "file";
    proc_panic(msg, file, 17);
    EXPECT_EQ(_sys_proc_panic_fake.call_count, 1);
    EXPECT_EQ((void *)_sys_proc_panic_fake.arg0_val, msg);
    EXPECT_EQ((void *)_sys_proc_panic_fake.arg1_val, file);
    EXPECT_EQ(_sys_proc_panic_fake.arg2_val, 17);
}
