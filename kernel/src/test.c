#include "test.h"

#include "libc/circbuff.h"
#include "test_lib.h"

int test_circbuff_new() {
    ASSERT_EQ(0, circbuff_new(0));

    circbuff_t * cbuff = circbuff_new(4);
    ASSERT_NOT_EQ(0, cbuff);
    ASSERT_EQ(0, circbuff_len(cbuff));
    ASSERT_EQ(4, circbuff_buff_size(cbuff));

    circbuff_free(cbuff);
    return 0;
}

#define NEW_BUFFER(N)                  \
    circbuff_new((N));                 \
    ASSERT_NOT_EQ(0, cbuff);           \
    ASSERT_EQ(0, circbuff_len(cbuff)); \
    ASSERT_EQ((N), circbuff_buff_size(cbuff));

int test_circbuff_at() {
    circbuff_t * cbuff = NEW_BUFFER(3);

    ASSERT_EQ(0, circbuff_at(cbuff, 0));
    ASSERT_EQ(0, circbuff_at(cbuff, 1));

    ASSERT_EQ(1, circbuff_push(cbuff, 1)); // [1, x, x]
    ASSERT_EQ(1, circbuff_at(cbuff, 0));

    ASSERT_EQ(1, circbuff_push(cbuff, 2)); // [1, 2, x]
    ASSERT_EQ(2, circbuff_at(cbuff, 1));

    ASSERT_EQ(1, circbuff_pop(cbuff)); // [x, 2, x]
    ASSERT_EQ(2, circbuff_at(cbuff, 0));

    ASSERT_EQ(1, circbuff_push(cbuff, 3)); // [x, 2, 3]
    ASSERT_EQ(3, circbuff_at(cbuff, 1));

    // at will wrap index around
    ASSERT_EQ(1, circbuff_push(cbuff, 4)); // [4, 2, 3]
    ASSERT_EQ(4, circbuff_at(cbuff, 2));

    circbuff_free(cbuff);
    return 0;
}

int test_circbuff_push_pop() {
    circbuff_t * cbuff = NEW_BUFFER(2);

    ASSERT_EQ(0, circbuff_at(cbuff, 0));
    ASSERT_EQ(0, circbuff_at(cbuff, 1));

    ASSERT_EQ(1, circbuff_push(cbuff, 1)); // [1, x]
    ASSERT_EQ(1, circbuff_len(cbuff));
    ASSERT_EQ(1, circbuff_at(cbuff, 0));

    ASSERT_EQ(1, circbuff_push(cbuff, 2)); // [1, 2]
    ASSERT_EQ(2, circbuff_len(cbuff));
    ASSERT_EQ(1, circbuff_at(cbuff, 0));
    ASSERT_EQ(2, circbuff_at(cbuff, 1));

    ASSERT_EQ(0, circbuff_push(cbuff, 3)); // [1, 2]
    ASSERT_EQ(2, circbuff_len(cbuff));
    ASSERT_EQ(1, circbuff_at(cbuff, 0));
    ASSERT_EQ(2, circbuff_at(cbuff, 1));

    ASSERT_EQ(1, circbuff_pop(cbuff)); // [x, 2]
    ASSERT_EQ(1, circbuff_len(cbuff));
    ASSERT_EQ(2, circbuff_at(cbuff, 0));

    ASSERT_EQ(1, circbuff_push(cbuff, 3)); // [3, 2]
    ASSERT_EQ(2, circbuff_len(cbuff));
    ASSERT_EQ(2, circbuff_at(cbuff, 0));
    ASSERT_EQ(3, circbuff_at(cbuff, 1));

    ASSERT_EQ(0, circbuff_push(cbuff, 4)); // [3, 2]
    ASSERT_EQ(2, circbuff_len(cbuff));
    ASSERT_EQ(2, circbuff_at(cbuff, 0));
    ASSERT_EQ(3, circbuff_at(cbuff, 1));

    ASSERT_EQ(2, circbuff_pop(cbuff)); // [3, x]
    ASSERT_EQ(1, circbuff_len(cbuff));
    ASSERT_EQ(3, circbuff_at(cbuff, 0));

    ASSERT_EQ(3, circbuff_pop(cbuff)); // [x, x]
    ASSERT_EQ(0, circbuff_len(cbuff));

    ASSERT_EQ(0, circbuff_pop(cbuff)); // [x, x]
    ASSERT_EQ(0, circbuff_len(cbuff));

    circbuff_free(cbuff);
    return 0;
}

int test_Circbuff() {
    circbuff_t * cbuff = circbuff_new(4);
    ASSERT_NOT_EQ(0, cbuff);
    ASSERT_EQ(0, circbuff_len(cbuff));
    ASSERT_EQ(4, circbuff_buff_size(cbuff));

    ASSERT_EQ(0, circbuff_at(cbuff, 0));
    ASSERT_EQ(0, circbuff_at(cbuff, 1));

    ASSERT_EQ(1, circbuff_push(cbuff, 4));
    ASSERT_EQ(4, circbuff_at(cbuff, 0));

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

    uint8_t out[4] = {0};
    ASSERT_EQ(3, circbuff_read(cbuff, out, 3));
    ASSERT_EQ(3, circbuff_len(cbuff));
    for (size_t i = 0; i < 3; i++) {
        ASSERT_EQ(tmp[i], out[i]);
    }

    ASSERT_EQ(3, circbuff_remove(cbuff, out, 3));
    ASSERT_EQ(0, circbuff_len(cbuff));
    for (size_t i = 0; i < 3; i++) {
        ASSERT_EQ(tmp[i], out[i]);
    }

    ASSERT_EQ(0, circbuff_remove(cbuff, out, 3));
    ASSERT_EQ(0, circbuff_len(cbuff));

    // TODO: edge case for insert, read and remove

    ASSERT_EQ(3, circbuff_insert(cbuff, tmp, 3));
    ASSERT_EQ(3, circbuff_len(cbuff));
    ASSERT_EQ(0, circbuff_remove(cbuff, 0, 3));
    ASSERT_EQ(0, circbuff_len(cbuff));

    circbuff_free(cbuff);
    return 0;
}

int run_tests() {
    BEGIN_TESTS
    TEST(test_circbuff_new)
    TEST(test_circbuff_at)
    TEST(test_circbuff_push_pop)
    TEST(test_Circbuff)
    END_TESTS
}
