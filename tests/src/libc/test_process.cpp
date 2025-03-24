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

TEST_F(LibC, queue_event) {
    ebus_event_t event;
    memset(&event, 0, sizeof(ebus_event_t));
    queue_event(&event);
    ASSERT_EQ(1, _sys_queue_event_fake.call_count);
    EXPECT_EQ(&event, _sys_queue_event_fake.arg0_val);
}

TEST_F(LibC, pull_event) {
    _sys_yield_fake.return_val = 3;
    ebus_event_t event;
    EXPECT_EQ(3, pull_event(1, &event));
    ASSERT_EQ(1, _sys_yield_fake.call_count);
    EXPECT_EQ(1, _sys_yield_fake.arg0_val);
    EXPECT_EQ(&event, _sys_yield_fake.arg1_val);
}

TEST_F(LibC, yield) {
    _sys_yield_fake.return_val = 2;
    yield();
    ASSERT_EQ(1, _sys_yield_fake.call_count);
    EXPECT_EQ(0, _sys_yield_fake.arg0_val);
    EXPECT_EQ(0, _sys_yield_fake.arg1_val);
}

TEST_F(LibC, getpid) {
    _sys_proc_getpid_fake.return_val = 2;
    EXPECT_EQ(2, getpid());
    ASSERT_EQ(1, _sys_proc_getpid_fake.call_count);
}
