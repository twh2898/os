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
    // FATAL();
    return 0;
}

BEGIN_TESTS
TEST(test_First);
END_TESTS
```
*/

#include <stdio.h>
#include <stdlib.h>

#define BEGIN_TESTS   \
    int main() {      \
        int fail = 0; \
        int total = 0;

#define END_TESTS                                    \
    if (fail) {                                      \
        int pass = total - fail;                     \
        printf("%d tests ran\n", total);             \
        printf("%d passed %d failed\n", pass, fail); \
    }                                                \
    return fail;                                     \
    }

#define TEST(NAME)                \
    {                             \
        printf("%10s : ", #NAME); \
        fflush(stdout);           \
        int err = NAME();         \
        if (err)                  \
            puts(": FAIL");       \
        else                      \
            puts("PASS");         \
        fail += err;              \
        total++;                  \
    }

#define ASSERT(expr)                                               \
    if (!(expr)) {                                                 \
        fprintf(stderr, "%s (%s:%d) ", #expr, __func__, __LINE__); \
        return 1;                                                  \
    }
#define ASSERT_EQ(a, b)     ASSERT((a) == (b))
#define ASSERT_NOT_EQ(a, b) ASSERT((a) != (b))
#define ASSERT_TRUE(a)      ASSERT((a) == true)
#define ASSERT_FALSE(a)     ASSERT((a) == false)
#define FATAL()                                                      \
    {                                                                \
        fprintf(stderr, "FATAL ERROR! %s:%d\n", __func__, __LINE__); \
        exit(-1);                                                    \
    }
