#include <cstdlib>
#include <vector>

#include "test_common.h"

extern "C" {
#include <string.h>

#include "libc/datastruct/circular_buffer.h"
}

class CircularBuffer : public testing::Test {
protected:
    cb_t cbuff;

    void SetUp() override {
        init_mocks();

        cb_create(&cbuff, 3, 1);

        RESET_FAKE(pmalloc);
        pmalloc_fake.custom_fake = malloc;
    }

    void TearDown() override {
        cb_free(&cbuff);
    }
};

TEST_F(CircularBuffer, cb_create) {
    // Bad args
    EXPECT_NE(0, cb_create(&cbuff, 0, 0));
    EXPECT_NE(0, cb_create(0, 1, 0));
    EXPECT_NE(0, cb_create(0, 0, 1));
    EXPECT_NE(0, cb_create(&cbuff, 1, 0));
    EXPECT_NE(0, cb_create(&cbuff, 0, 1));
    EXPECT_EQ(0, pmalloc_fake.call_count);
    EXPECT_EQ(0, pfree_fake.call_count);

    // Good args
    EXPECT_EQ(0, cb_create(&cbuff, 4, 1));
    EXPECT_EQ(0, cb_len(&cbuff));
    EXPECT_EQ(4, cb_buff_size(&cbuff));

    ASSERT_EQ(1, pmalloc_fake.call_count);
    EXPECT_EQ(4, pmalloc_fake.arg0_val);

    cb_free(&cbuff);

    SetUp();

    // Larger elements
    EXPECT_EQ(0, cb_create(&cbuff, 4, 2));
    EXPECT_EQ(0, cb_len(&cbuff));
    EXPECT_EQ(4, cb_buff_size(&cbuff));

    ASSERT_EQ(1, pmalloc_fake.call_count);
    EXPECT_EQ(8, pmalloc_fake.arg0_val);

    cb_free(&cbuff);

    SetUp();

    pmalloc_fake.custom_fake = 0;
    pmalloc_fake.return_val  = 0;

    // First malloc fails
    EXPECT_NE(0, cb_create(&cbuff, 4, 1));
    EXPECT_EQ(1, pmalloc_fake.call_count);
    EXPECT_EQ(4, pmalloc_fake.arg0_val);
}

TEST_F(CircularBuffer, cb_free) {
    cb_free(0);
    EXPECT_EQ(0, pfree_fake.call_count);

    void * data = cbuff.buff;
    cb_free(&cbuff);
    EXPECT_EQ(1, pfree_fake.call_count);
    EXPECT_EQ(data, pfree_fake.arg0_val);
    EXPECT_EQ(0, cbuff.buff);
}

TEST_F(CircularBuffer, cb_buff_size) {
    EXPECT_EQ(3, cb_buff_size(&cbuff));
    cb_free(&cbuff);

    EXPECT_EQ(0, cb_create(&cbuff, 3, 2));
    EXPECT_EQ(3, cb_buff_size(&cbuff));
}

TEST_F(CircularBuffer, cb_len) {
    // Empty
    EXPECT_EQ(0, cb_len(&cbuff));

    // 1 element
    char c = '1';
    EXPECT_EQ(0, cb_push(&cbuff, &c));
    EXPECT_EQ(1, cb_len(&cbuff));

    // 2 element
    EXPECT_EQ(0, cb_push(&cbuff, &c));
    EXPECT_EQ(2, cb_len(&cbuff));

    // 3 element
    EXPECT_EQ(0, cb_push(&cbuff, &c));
    EXPECT_EQ(3, cb_len(&cbuff));

    // too many
    EXPECT_NE(0, cb_push(&cbuff, &c));
    EXPECT_EQ(3, cb_len(&cbuff));
}

TEST_F(CircularBuffer, cb_peek) {
    // Invalid Parameters
    EXPECT_EQ(0, cb_peek(0, 0));

    // No content
    EXPECT_EQ(0, cb_peek(&cbuff, 0));
    EXPECT_EQ(0, cb_peek(&cbuff, 1));
    EXPECT_EQ(0, cb_peek(&cbuff, 2));
    EXPECT_EQ(0, cb_peek(&cbuff, 3));

    char a = '1';
    EXPECT_EQ(0, cb_push(&cbuff, &a));

    char   b = '1';
    void * c = cb_peek(&cbuff, 0);
    EXPECT_NE(0, b);
    EXPECT_EQ('1', *((char *)c));
    EXPECT_EQ(0, cb_peek(&cbuff, 1));
    EXPECT_EQ(0, cb_peek(&cbuff, 2));

    cb_free(&cbuff);

    EXPECT_EQ(0, cb_create(&cbuff, 2, 4));
    EXPECT_EQ(0, cb_len(&cbuff));
    EXPECT_EQ(2, cb_buff_size(&cbuff));

    // No elements
    EXPECT_EQ(0, cb_peek(&cbuff, 0));
    EXPECT_EQ(0, cb_peek(&cbuff, 1));

    // Read past end
    EXPECT_EQ(0, cb_peek(&cbuff, 2));

    int i1 = 12345;
    ASSERT_EQ(0, cb_push(&cbuff, &i1));

    // 1 element
    int * i2 = (int *)cb_peek(&cbuff, 0);
    ASSERT_NE(nullptr, i2);
    EXPECT_EQ(i1, *i2);
    EXPECT_EQ(0, cb_peek(&cbuff, 1));

    int i3 = 6789;
    ASSERT_EQ(0, cb_push(&cbuff, &i3));

    // 2 element
    i2 = (int *)cb_peek(&cbuff, 0);
    ASSERT_NE(nullptr, i2);
    EXPECT_EQ(i1, *i2);

    i2 = (int *)cb_peek(&cbuff, 1);
    ASSERT_NE(nullptr, i2);
    EXPECT_EQ(i3, *i2);
}

TEST_F(CircularBuffer, cb_push) {
    // Invalid Parameter
    EXPECT_NE(0, cb_push(0, 0));
    EXPECT_NE(0, cb_push(&cbuff, 0));

    // Good
    EXPECT_EQ(0, cb_len(&cbuff));
    ASSERT_EQ(3, cb_buff_size(&cbuff));

    char c = '1';
    EXPECT_EQ(0, cb_push(&cbuff, &c));
    EXPECT_EQ(1, cb_len(&cbuff));

    EXPECT_EQ(0, cb_push(&cbuff, &c));
    EXPECT_EQ(2, cb_len(&cbuff));

    EXPECT_EQ(0, cb_push(&cbuff, &c));
    EXPECT_EQ(3, cb_len(&cbuff));

    EXPECT_NE(0, cb_push(&cbuff, &c));
    EXPECT_EQ(3, cb_len(&cbuff));

    EXPECT_EQ(3, cb_buff_size(&cbuff));
}

TEST_F(CircularBuffer, cb_pop) {
    EXPECT_EQ(0, cb_len(&cbuff));
    ASSERT_EQ(3, cb_buff_size(&cbuff));

    char item = 0;

    // Pop with no elements
    EXPECT_NE(0, cb_pop(&cbuff, 0));
    EXPECT_NE(0, cb_pop(&cbuff, &item));

    // Fill
    char c = 'a';
    ASSERT_EQ(0, cb_push(&cbuff, &c));
    c = 'b';
    ASSERT_EQ(0, cb_push(&cbuff, &c));
    c = 'c';
    ASSERT_EQ(0, cb_push(&cbuff, &c));

    // Invalid Parameter
    ASSERT_NE(0, cb_pop(0, &item));

    // Check filled
    ASSERT_EQ(3, cb_len(&cbuff));

    // Pop each element with item
    EXPECT_EQ(0, cb_pop(&cbuff, &c));
    EXPECT_EQ('a', c);

    EXPECT_EQ(0, cb_pop(&cbuff, &c));
    EXPECT_EQ('b', c);

    EXPECT_EQ(0, cb_pop(&cbuff, &c));
    EXPECT_EQ('c', c);

    // Too many
    c = '3';
    EXPECT_NE(0, cb_pop(&cbuff, &c));
    // Not changed
    EXPECT_EQ('3', c);

    // Fill
    c = 'a';
    ASSERT_EQ(0, cb_push(&cbuff, &c));
    c = 'b';
    ASSERT_EQ(0, cb_push(&cbuff, &c));
    c = 'c';
    ASSERT_EQ(0, cb_push(&cbuff, &c));

    // Check filled
    ASSERT_EQ(3, cb_len(&cbuff));

    // Pop each element no item
    EXPECT_EQ(0, cb_pop(&cbuff, 0));
    EXPECT_EQ(0, cb_pop(&cbuff, 0));
    EXPECT_EQ(0, cb_pop(&cbuff, 0));

    // One too many
    EXPECT_NE(0, cb_pop(&cbuff, 0));
}

TEST_F(CircularBuffer, cb_rpop) {
    EXPECT_EQ(0, cb_len(&cbuff));
    ASSERT_EQ(3, cb_buff_size(&cbuff));

    char item = 0;

    // Pop with no elements
    EXPECT_NE(0, cb_rpop(&cbuff, 0));
    EXPECT_NE(0, cb_rpop(&cbuff, &item));

    // Fill
    char c = 'a';
    ASSERT_EQ(0, cb_push(&cbuff, &c));
    c = 'b';
    ASSERT_EQ(0, cb_push(&cbuff, &c));
    c = 'c';
    ASSERT_EQ(0, cb_push(&cbuff, &c));

    // Invalid Parameter
    ASSERT_NE(0, cb_rpop(0, &item));

    // Check filled
    ASSERT_EQ(3, cb_len(&cbuff));

    // Pop each element with item
    EXPECT_EQ(0, cb_rpop(&cbuff, &c));
    EXPECT_EQ('c', c);

    EXPECT_EQ(0, cb_rpop(&cbuff, &c));
    EXPECT_EQ('b', c);

    EXPECT_EQ(0, cb_rpop(&cbuff, &c));
    EXPECT_EQ('a', c);

    // Too many
    c = '3';
    EXPECT_NE(0, cb_rpop(&cbuff, &c));
    // Not changed
    EXPECT_EQ('3', c);

    // Fill
    c = 'a';
    ASSERT_EQ(0, cb_push(&cbuff, &c));
    c = 'b';
    ASSERT_EQ(0, cb_push(&cbuff, &c));
    c = 'c';
    ASSERT_EQ(0, cb_push(&cbuff, &c));

    // Check filled
    ASSERT_EQ(3, cb_len(&cbuff));

    // Pop each element no item
    EXPECT_EQ(0, cb_rpop(&cbuff, 0));
    EXPECT_EQ(0, cb_rpop(&cbuff, 0));
    EXPECT_EQ(0, cb_rpop(&cbuff, 0));

    // One too many
    EXPECT_NE(0, cb_rpop(&cbuff, 0));
}
