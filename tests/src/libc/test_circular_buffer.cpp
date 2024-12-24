#include <cstdlib>
#include <vector>

#include "test_header.h"

extern "C" {
#include <string.h>

#include "libc/datastruct/circular_buffer.h"
}

extern "C" {
FAKE_VALUE_FUNC(void *, kmalloc, size_t);
FAKE_VOID_FUNC(kfree, void *);
FAKE_VALUE_FUNC(void *, kmemcpy, void *, const void *, size_t);
}

static void setup_fakes() {
    RESET_FAKE(kmalloc);
    RESET_FAKE(kfree);
    RESET_FAKE(kmemcpy);

    kmalloc_fake.custom_fake = malloc;
    kfree_fake.custom_fake   = free;
    kmemcpy_fake.custom_fake = memcpy;
}

TEST(CircularBufferStatic, cb_new) {
    setup_fakes();

    // Bad args
    EXPECT_EQ(0, cb_new(0, 0));
    EXPECT_EQ(0, cb_new(1, 0));
    EXPECT_EQ(0, cb_new(0, 1));
    EXPECT_EQ(0, kmalloc_fake.call_count);
    EXPECT_EQ(0, kfree_fake.call_count);

    // Good args
    cb_t * cbuff = cb_new(4, 1);
    ASSERT_NE(nullptr, cbuff);
    EXPECT_EQ(0, cb_len(cbuff));
    EXPECT_EQ(4, cb_buff_size(cbuff));

    ASSERT_EQ(2, kmalloc_fake.call_count);
    EXPECT_EQ(20, kmalloc_fake.arg0_history[0]);
    EXPECT_EQ(4, kmalloc_fake.arg0_history[1]);

    cb_free(cbuff);

    setup_fakes();

    // Larger elements
    cbuff = cb_new(4, 2);
    ASSERT_NE(nullptr, cbuff);
    EXPECT_EQ(0, cb_len(cbuff));
    EXPECT_EQ(4, cb_buff_size(cbuff));

    ASSERT_EQ(2, kmalloc_fake.call_count);
    EXPECT_EQ(20, kmalloc_fake.arg0_history[0]);
    EXPECT_EQ(8, kmalloc_fake.arg0_history[1]);

    cb_free(cbuff);

    setup_fakes();
    kmalloc_fake.custom_fake = 0;
    kmalloc_fake.return_val  = 0;

    // First malloc fails
    EXPECT_EQ(0, cb_new(4, 1));
    EXPECT_EQ(1, kmalloc_fake.call_count);
    EXPECT_EQ(20, kmalloc_fake.arg0_val);

    void * ret_seq[2] = {malloc(20), 0};

    setup_fakes();
    kmalloc_fake.custom_fake = 0;
    SET_RETURN_SEQ(kmalloc, ret_seq, 2);

    // Second malloc fails
    EXPECT_EQ(0, cb_new(4, 1));
    EXPECT_EQ(2, kmalloc_fake.call_count);
    EXPECT_EQ(20, kmalloc_fake.arg0_history[0]);
    EXPECT_EQ(4, kmalloc_fake.arg0_history[1]);
    EXPECT_EQ(1, kfree_fake.call_count);
    EXPECT_EQ(ret_seq[0], kfree_fake.arg0_val);
}

class CircularBuffer : public testing::Test {
protected:
    cb_t * cbuff;

    void SetUp() override {
        setup_fakes();

        cbuff = cb_new(3, 1);

        ASSERT_NE(nullptr, cbuff);
        ASSERT_EQ(0, cb_len(cbuff));
        ASSERT_EQ(3, cb_buff_size(cbuff));

        RESET_FAKE(kmalloc);
        kmalloc_fake.custom_fake = malloc;
    }

    void TearDown() override {
        if (cbuff) {
            cb_free(cbuff);
        }
    }
};

TEST_F(CircularBuffer, cb_free) {
    cb_free(0);
    EXPECT_EQ(0, kfree_fake.call_count);

    cb_free(cbuff);
    EXPECT_EQ(2, kfree_fake.call_count);
    EXPECT_EQ(cbuff, kfree_fake.arg0_history[1]);

    cbuff = 0;
}

TEST_F(CircularBuffer, cb_buff_size) {
    EXPECT_EQ(3, cb_buff_size(cbuff));
    cb_free(cbuff);

    cbuff = cb_new(3, 2);
    EXPECT_EQ(3, cb_buff_size(cbuff));
}

TEST_F(CircularBuffer, cb_len) {
    // Empty
    EXPECT_EQ(0, cb_len(cbuff));

    // 1 element
    char c = '1';
    EXPECT_EQ(0, cb_push(cbuff, &c));
    EXPECT_EQ(1, cb_len(cbuff));

    // 2 element
    EXPECT_EQ(0, cb_push(cbuff, &c));
    EXPECT_EQ(2, cb_len(cbuff));

    // 3 element
    EXPECT_EQ(0, cb_push(cbuff, &c));
    EXPECT_EQ(3, cb_len(cbuff));

    // too many
    EXPECT_NE(0, cb_push(cbuff, &c));
    EXPECT_EQ(3, cb_len(cbuff));
}

TEST_F(CircularBuffer, cb_peek) {
    EXPECT_EQ(0, cb_peek(cbuff, 0));
    EXPECT_EQ(0, cb_peek(cbuff, 1));
    EXPECT_EQ(0, cb_peek(cbuff, 2));
    EXPECT_EQ(0, cb_peek(cbuff, 3));

    char a = '1';
    EXPECT_EQ(0, cb_push(cbuff, &a));

    char   b = '1';
    void * c = cb_peek(cbuff, 0);
    EXPECT_NE(0, b);
    EXPECT_EQ('1', *((char *)c));
    EXPECT_EQ(0, cb_peek(cbuff, 1));
    EXPECT_EQ(0, cb_peek(cbuff, 2));

    cb_free(cbuff);

    cbuff = cb_new(2, 4);
    ASSERT_NE(nullptr, cbuff);
    EXPECT_EQ(0, cb_len(cbuff));
    EXPECT_EQ(2, cb_buff_size(cbuff));

    // No elements
    EXPECT_EQ(0, cb_peek(cbuff, 0));
    EXPECT_EQ(0, cb_peek(cbuff, 1));

    // Read past end
    EXPECT_EQ(0, cb_peek(cbuff, 2));

    int i1 = 12345;
    ASSERT_EQ(0, cb_push(cbuff, &i1));

    // 1 element
    int * i2 = (int *)cb_peek(cbuff, 0);
    ASSERT_NE(nullptr, i2);
    EXPECT_EQ(i1, *i2);
    EXPECT_EQ(0, cb_peek(cbuff, 1));

    int i3 = 6789;
    ASSERT_EQ(0, cb_push(cbuff, &i3));

    // 2 element
    i2 = (int *)cb_peek(cbuff, 0);
    ASSERT_NE(nullptr, i2);
    EXPECT_EQ(i1, *i2);

    i2 = (int *)cb_peek(cbuff, 1);
    ASSERT_NE(nullptr, i2);
    EXPECT_EQ(i3, *i2);
}

TEST_F(CircularBuffer, cb_push) {
    EXPECT_EQ(0, cb_len(cbuff));
    ASSERT_EQ(3, cb_buff_size(cbuff));

    char c = '1';
    EXPECT_EQ(0, cb_push(cbuff, &c));
    EXPECT_EQ(1, cb_len(cbuff));

    EXPECT_EQ(0, cb_push(cbuff, &c));
    EXPECT_EQ(2, cb_len(cbuff));

    EXPECT_EQ(0, cb_push(cbuff, &c));
    EXPECT_EQ(3, cb_len(cbuff));

    EXPECT_NE(0, cb_push(cbuff, &c));
    EXPECT_EQ(3, cb_len(cbuff));

    EXPECT_EQ(3, cb_buff_size(cbuff));
}

TEST_F(CircularBuffer, cb_pop) {
    EXPECT_EQ(0, cb_len(cbuff));
    ASSERT_EQ(3, cb_buff_size(cbuff));

    char item = 0;

    // Pop with no elements
    EXPECT_NE(0, cb_pop(cbuff, 0));
    EXPECT_NE(0, cb_pop(cbuff, &item));

    // Fill
    char c = 'a';
    ASSERT_EQ(0, cb_push(cbuff, &c));
    c = 'b';
    ASSERT_EQ(0, cb_push(cbuff, &c));
    c = 'c';
    ASSERT_EQ(0, cb_push(cbuff, &c));

    // Check filled
    ASSERT_EQ(3, cb_len(cbuff));

    // Pop each element with item
    EXPECT_EQ(0, cb_pop(cbuff, &c));
    EXPECT_EQ('a', c);

    EXPECT_EQ(0, cb_pop(cbuff, &c));
    EXPECT_EQ('b', c);

    EXPECT_EQ(0, cb_pop(cbuff, &c));
    EXPECT_EQ('c', c);

    // Too many
    c = '3';
    EXPECT_NE(0, cb_pop(cbuff, &c));
    // Not changed
    EXPECT_EQ('3', c);

    // Fill
    c = 'a';
    ASSERT_EQ(0, cb_push(cbuff, &c));
    c = 'b';
    ASSERT_EQ(0, cb_push(cbuff, &c));
    c = 'c';
    ASSERT_EQ(0, cb_push(cbuff, &c));

    // Check filled
    ASSERT_EQ(3, cb_len(cbuff));

    // Pop each element no item
    EXPECT_EQ(0, cb_pop(cbuff, 0));
    EXPECT_EQ(0, cb_pop(cbuff, 0));
    EXPECT_EQ(0, cb_pop(cbuff, 0));

    // One too many
    EXPECT_NE(0, cb_pop(cbuff, 0));

    TearDown();
    SetUp();

    // Fill
    c = 'a';
    ASSERT_EQ(0, cb_push(cbuff, &c));
    c = 'b';
    ASSERT_EQ(0, cb_push(cbuff, &c));
    c = 'c';
    ASSERT_EQ(0, cb_push(cbuff, &c));

    // Check filled
    ASSERT_EQ(3, cb_len(cbuff));

    kmemcpy_fake.custom_fake = 0;
    kmemcpy_fake.return_val  = 0;

    // Does not change
    c = '3';
    ASSERT_NE(0, cb_pop(cbuff, &c));
    EXPECT_EQ('3', c);
}
