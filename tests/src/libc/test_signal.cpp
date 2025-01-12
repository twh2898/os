#include <cstdlib>
#include <cstring>
#include <string>

#include "test_common.h"

extern "C" {
#include "libc/signal.h"

FAKE_VOID_FUNC(callback);
}

typedef void (*signal_callback_t)(int);

TEST(Signal, register_signal) {
    init_mocks();

    RESET_FAKE(_register_signals);
    RESET_FAKE(callback);

    kmalloc_fake.custom_fake = malloc;

    EXPECT_NE(0, register_signal(0, 0));

    EXPECT_EQ(0, kmalloc_fake.call_count);
    EXPECT_EQ(0, _register_signals_fake.call_count);

    EXPECT_EQ(0, register_signal(0, callback));
    EXPECT_EQ(1, kmalloc_fake.call_count);
    EXPECT_EQ(12, kmalloc_fake.arg0_val);

    signal_callback_t signal_callback = (signal_callback_t)_register_signals_fake.arg0_val;

    signal_callback(0);
    EXPECT_EQ(1, callback_fake.call_count);

    signal_callback(1);
    EXPECT_EQ(1, callback_fake.call_count);

    RESET_FAKE(kmalloc);
    kmalloc_fake.custom_fake = malloc;

    EXPECT_NE(0, register_signal(0, callback));
    EXPECT_EQ(0, kmalloc_fake.call_count);
}
