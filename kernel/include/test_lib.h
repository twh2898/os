#pragma once

/*
C/C++ single header testing library.

Author: Thomas Harrison <twh2898@vt.edu>
Version: 0.0.3
URL: https://gist.github.com/twh2898/c01a70a0a99adcb14d974eeb9529e0fb

```
#include "test.h"

int test_First() {
    int a = 1;
    int b = a + 2;
    ASSERT_EQ(a + 2, b);
    ASSERT_NOT_EQ(a + 1, b);
    ASSERT_NOT_EQ(a, b);
    ASSERT_TRUE(a > 0);
    ASSERT_FALSE(a > b);
    // FATAL;
    return 0;
}

BEGIN_TESTS
TEST(test_First);
END_TESTS
```
*/

#include "libc/stdio.h"

#define BEGIN_TESTS \
    int fail = 0;   \
    int total = 0;

#define END_TESTS                                     \
    if (fail) {                                       \
        int pass = total - fail;                      \
        kprintf("%d tests ran\n", total);             \
        kprintf("%d passed %d failed\n", pass, fail); \
    }                                                 \
    return fail;

#define TEST(NAME)                 \
    {                              \
        kprintf("%10s : ", #NAME); \
        int err = NAME();          \
        if (err)                   \
            kputs(": FAIL");       \
        else                       \
            kputs("PASS");         \
        fail += err;               \
        total++;                   \
        kputc('\n');               \
    }

#define ASSERT(expr)                                       \
    if (!(expr)) {                                         \
        kprintf("%s (%s:%d) ", #expr, __func__, __LINE__); \
        return 1;                                          \
    }
#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NOT_EQ(a, b) ASSERT((a) != (b))
#define ASSERT_TRUE(a) ASSERT((a) == true)
#define ASSERT_FALSE(a) ASSERT((a) == false)
#define FATAL                                                \
    {                                                        \
        kprintf("FATAL ERROR! %s:%d\n", __func__, __LINE__); \
        return -1;                                           \
    }
