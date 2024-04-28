#include "test.h"

#include "libc/circbuff.h"
#include "test_lib.h"

int test_Circbuff() {
    circbuff_t * cbuff = circbuff_new(4);
    ASSERT_NOT_EQ(0, cbuff);
    ASSERT_EQ(0, circbuff_len(cbuff));
    ASSERT_EQ(4, circbuff_buff_size(cbuff));

    ASSERT_EQ(1, circbuff_push(cbuff, 1));
    ASSERT_EQ(1, circbuff_len(cbuff));

    ASSERT_EQ(1, circbuff_push(cbuff, 3));
    ASSERT_EQ(2, circbuff_len(cbuff));

    ASSERT_EQ(1, circbuff_at(cbuff, 0));
    ASSERT_EQ(3, circbuff_at(cbuff, 1));

    ASSERT_EQ(1, circbuff_pop(cbuff));
    ASSERT_EQ(1, circbuff_len(cbuff));
    ASSERT_EQ(3, circbuff_at(cbuff, 0));

    // TODO: edge case for at, push and pop

    size_t len = circbuff_len(cbuff);
    ASSERT(len > 0);
    ASSERT_EQ(len, circbuff_clear(cbuff));
    ASSERT_EQ(0, circbuff_len(cbuff));

    uint8_t tmp[4] = {1, 2, 3, 4};
    ASSERT_EQ(3, circbuff_insert(cbuff, tmp, 3));
    ASSERT_EQ(3, circbuff_len(cbuff));
    for (size_t i = 0; i < 3; i++) {
        ASSERT_EQ(tmp[i], circbuff_at(cbuff, i));
    }

    circbuff_free(cbuff);
    return 0;
}

int run_tests() {
    BEGIN_TESTS
    TEST(test_Circbuff)
    END_TESTS
}
