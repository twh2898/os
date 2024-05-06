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

    // TODO test circbuff_rpop

    circbuff_free(cbuff);
    return 0;
}

int test_circbuff_insert() {
    uint8_t tmp[4] = {1, 2, 3, 4};
    circbuff_t * cbuff = NEW_BUFFER(4);

    // insert 0
    ASSERT_EQ(0, circbuff_insert(cbuff, tmp, 0));
    ASSERT_EQ(0, circbuff_len(cbuff));

    // insert 1
    ASSERT_EQ(1, circbuff_insert(cbuff, tmp, 1));
    ASSERT_EQ(1, circbuff_len(cbuff));
    ASSERT_EQ(tmp[0], circbuff_at(cbuff, 0));

    // insert 1+
    ASSERT_EQ(3, circbuff_insert(cbuff, tmp, 3));
    ASSERT_EQ(4, circbuff_len(cbuff));
    ASSERT_EQ(tmp[0], circbuff_at(cbuff, 0));
    ASSERT_EQ(tmp[0], circbuff_at(cbuff, 1));
    ASSERT_EQ(tmp[1], circbuff_at(cbuff, 2));
    ASSERT_EQ(tmp[2], circbuff_at(cbuff, 3));

    // insert 0 full
    ASSERT_EQ(0, circbuff_insert(cbuff, tmp, 0));
    ASSERT_EQ(4, circbuff_len(cbuff));
    ASSERT_EQ(tmp[0], circbuff_at(cbuff, 0));
    ASSERT_EQ(tmp[0], circbuff_at(cbuff, 1));
    ASSERT_EQ(tmp[1], circbuff_at(cbuff, 2));
    ASSERT_EQ(tmp[2], circbuff_at(cbuff, 3));

    // insert 1 full
    ASSERT_EQ(0, circbuff_insert(cbuff, tmp, 1));
    ASSERT_EQ(4, circbuff_len(cbuff));
    ASSERT_EQ(tmp[0], circbuff_at(cbuff, 0));
    ASSERT_EQ(tmp[0], circbuff_at(cbuff, 1));
    ASSERT_EQ(tmp[1], circbuff_at(cbuff, 2));
    ASSERT_EQ(tmp[2], circbuff_at(cbuff, 3));

    // insert 1+ full
    ASSERT_EQ(0, circbuff_insert(cbuff, tmp, 2));
    ASSERT_EQ(4, circbuff_len(cbuff));
    ASSERT_EQ(tmp[0], circbuff_at(cbuff, 0));
    ASSERT_EQ(tmp[0], circbuff_at(cbuff, 1));
    ASSERT_EQ(tmp[1], circbuff_at(cbuff, 2));
    ASSERT_EQ(tmp[2], circbuff_at(cbuff, 3));

    circbuff_free(cbuff);
    return 0;
}

int test_circbuff_read() {
    uint8_t tmp[4] = {1, 2, 3, 4};
    uint8_t out[4] = {0};
    circbuff_t * cbuff = NEW_BUFFER(4);

    // read 0 empty
    ASSERT_EQ(0, circbuff_read(cbuff, out, 0));

    // read 1 empty
    ASSERT_EQ(0, circbuff_read(cbuff, out, 1));

    // read 1+ empty
    ASSERT_EQ(0, circbuff_read(cbuff, out, 3));

    // read too much empty
    ASSERT_EQ(0, circbuff_read(cbuff, out, 8));

    // read 0
    ASSERT_EQ(4, circbuff_insert(cbuff, tmp, 4));
    ASSERT_EQ(0, circbuff_read(cbuff, out, 0));

    // read 1
    ASSERT_EQ(1, circbuff_read(cbuff, out, 1));
    ASSERT_EQ(tmp[0], out[0]);

    // read 1+
    ASSERT_EQ(3, circbuff_read(cbuff, out, 3));
    ASSERT_EQ(tmp[0], out[0]);
    ASSERT_EQ(tmp[1], out[1]);
    ASSERT_EQ(tmp[2], out[2]);

    // read too much
    ASSERT_EQ(4, circbuff_read(cbuff, out, 7));
    ASSERT_EQ(tmp[0], out[0]);
    ASSERT_EQ(tmp[1], out[1]);
    ASSERT_EQ(tmp[2], out[2]);
    ASSERT_EQ(tmp[3], out[3]);

    // TODO edge case read after remove where start has changed

    circbuff_free(cbuff);
    return 0;
}

int test_circbuff_remove() {
    uint8_t tmp[4] = {1, 2, 3, 4};
    uint8_t out[4] = {0};
    circbuff_t * cbuff = NEW_BUFFER(4);

    // remove 0 empty
    ASSERT_EQ(0, circbuff_remove(cbuff, 0));
    ASSERT_EQ(0, circbuff_len(cbuff));

    // remove 1 empty
    ASSERT_EQ(0, circbuff_remove(cbuff, 1));
    ASSERT_EQ(0, circbuff_len(cbuff));

    // remove 1+ empty
    ASSERT_EQ(0, circbuff_remove(cbuff, 3));
    ASSERT_EQ(0, circbuff_len(cbuff));

    // remove too much empty
    ASSERT_EQ(0, circbuff_remove(cbuff, 5));
    ASSERT_EQ(0, circbuff_len(cbuff));

    // remove 0
    ASSERT_EQ(4, circbuff_insert(cbuff, tmp, 4));
    ASSERT_EQ(0, circbuff_remove(cbuff, 0));
    ASSERT_EQ(4, circbuff_len(cbuff));
    ASSERT_EQ(tmp[0], circbuff_at(cbuff, 0));
    ASSERT_EQ(tmp[1], circbuff_at(cbuff, 1));
    ASSERT_EQ(tmp[2], circbuff_at(cbuff, 2));
    ASSERT_EQ(tmp[3], circbuff_at(cbuff, 3));

    // remove 1
    ASSERT_EQ(1, circbuff_remove(cbuff, 1));
    ASSERT_EQ(3, circbuff_len(cbuff));
    ASSERT_EQ(tmp[1], circbuff_at(cbuff, 0));
    ASSERT_EQ(tmp[2], circbuff_at(cbuff, 1));
    ASSERT_EQ(tmp[3], circbuff_at(cbuff, 2));

    // remove 1+
    ASSERT_EQ(3, circbuff_remove(cbuff, 3));
    ASSERT_EQ(0, circbuff_len(cbuff));

    // remove all
    ASSERT_EQ(4, circbuff_insert(cbuff, tmp, 4));
    ASSERT_EQ(4, circbuff_remove(cbuff, 4));
    ASSERT_EQ(0, circbuff_len(cbuff));

    // remove too much
    ASSERT_EQ(4, circbuff_insert(cbuff, tmp, 4));
    ASSERT_EQ(4, circbuff_remove(cbuff, 7));
    ASSERT_EQ(0, circbuff_len(cbuff));

    circbuff_free(cbuff);
    return 0;
}

int run_tests() {
    BEGIN_TESTS
    TEST(test_circbuff_new)
    TEST(test_circbuff_at)
    TEST(test_circbuff_push_pop)
    TEST(test_circbuff_insert)
    TEST(test_circbuff_read)
    TEST(test_circbuff_remove)
    END_TESTS
}
