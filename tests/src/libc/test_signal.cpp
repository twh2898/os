#include <cstdlib>
#include <cstring>
#include <string>

#include "test_common.h"

extern "C" {
#include "libc/signal.h"

FAKE_VOID_FUNC(callback);
}

typedef void (*signal_callback_t)(int);

class Signal : public testing::Test {
protected:
    void SetUp() override {
        init_mocks();
        RESET_FAKE(callback);
    }
};

TEST_F(Signal, register_signal) {
    // Invalid parameters
    EXPECT_NE(0, register_signal(0, 0));
    EXPECT_EQ(0, pmalloc_fake.call_count);
    EXPECT_EQ(0, _sys_register_signals_fake.call_count);

    // Success
    EXPECT_EQ(0, register_signal(0, callback));
    EXPECT_EQ(1, pmalloc_fake.call_count);
    EXPECT_EQ(12, pmalloc_fake.arg0_val);

    signal_callback_t signal_callback = (signal_callback_t)_sys_register_signals_fake.arg0_val;

    signal_callback(0);
    EXPECT_EQ(1, callback_fake.call_count);

    signal_callback(1);
    EXPECT_EQ(1, callback_fake.call_count);

    RESET_FAKE(pmalloc);
    pmalloc_fake.custom_fake = malloc;

    // Duplicate registered
    EXPECT_NE(0, register_signal(0, callback));
    EXPECT_EQ(0, pmalloc_fake.call_count);

    pmalloc_fake.custom_fake = 0;
    pmalloc_fake.return_val  = 0;

    // malloc fails
    EXPECT_NE(0, register_signal(1, callback));
}

// TEST_F(Signal, queue_event) {
//     ebus_event_t event;
//     event.event_id   = EBUS_EVENT_TIMER;
//     event.timer.time = 2;
//     queue_event(&event);
//     ASSERT_EQ(1, _sys_queue_event_fake.call_count);
//     EXPECT_EQ(&event, _sys_queue_event_fake.arg0_val);
// }
