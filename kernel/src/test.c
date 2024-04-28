#include "test.h"
#include "test_lib.h"

#include "libc/circbuff.h"

int test_Circbuff() {
    circbuff_t * cbuff = circbuff_new(8);
    ASSERT_NOT_EQ(0, cbuff);
    ASSERT_EQ(0, circbuff_len(cbuff));
    ASSERT_EQ(8, circbuff_buff_size(cbuff));
    return 0;
}

int run_tests() {
    BEGIN_TESTS
    TEST(test_Circbuff)
    END_TESTS
}
