#include <cstdlib>
#include <vector>

#include "test_common.h"

extern "C" {
#include <string.h>

#include "libc/datastruct/array.h"
}

static void setup_fakes() {
    init_mocks();
}

TEST(ArrayStatic, arr_new) {
    setup_fakes();

    // Bad args
    EXPECT_EQ(0, arr_new(0, 0));
    EXPECT_EQ(0, arr_new(1, 0));
    EXPECT_EQ(0, arr_new(0, 1));
    EXPECT_EQ(0, kmalloc_fake.call_count);
    EXPECT_EQ(0, kfree_fake.call_count);

    // Good args
    arr_t * arr = arr_new(4, 1);
    ASSERT_NE(nullptr, arr);
    EXPECT_EQ(0, arr_size(arr));

    ASSERT_EQ(2, kmalloc_fake.call_count);
    EXPECT_EQ(16, kmalloc_fake.arg0_history[0]);
    EXPECT_EQ(4, kmalloc_fake.arg0_history[1]);

    arr_free(arr);

    setup_fakes();

    // Larger elements
    arr = arr_new(4, 2);
    ASSERT_NE(nullptr, arr);
    EXPECT_EQ(0, arr_size(arr));

    ASSERT_EQ(2, kmalloc_fake.call_count);
    EXPECT_EQ(16, kmalloc_fake.arg0_history[0]);
    EXPECT_EQ(8, kmalloc_fake.arg0_history[1]);

    arr_free(arr);

    setup_fakes();
    kmalloc_fake.custom_fake = 0;
    kmalloc_fake.return_val  = 0;

    // First malloc fails
    EXPECT_EQ(0, arr_new(4, 1));
    EXPECT_EQ(1, kmalloc_fake.call_count);
    EXPECT_EQ(16, kmalloc_fake.arg0_val);

    void * ret_seq[2] = {malloc(16), 0};

    setup_fakes();
    kmalloc_fake.custom_fake = 0;
    SET_RETURN_SEQ(kmalloc, ret_seq, 2);

    // Second malloc fails
    EXPECT_EQ(0, arr_new(4, 1));
    EXPECT_EQ(2, kmalloc_fake.call_count);
    EXPECT_EQ(16, kmalloc_fake.arg0_history[0]);
    EXPECT_EQ(4, kmalloc_fake.arg0_history[1]);
    EXPECT_EQ(1, kfree_fake.call_count);
    EXPECT_EQ(ret_seq[0], kfree_fake.arg0_val);
}

class Array : public testing::Test {
protected:
    arr_t * arr;

    void SetUp() override {
        setup_fakes();

        arr = arr_new(3, 1);

        ASSERT_NE(nullptr, arr);
        ASSERT_EQ(0, arr_size(arr));

        RESET_FAKE(kmalloc);
        kmalloc_fake.custom_fake = malloc;
    }

    void TearDown() override {
        if (arr) {
            arr_free(arr);
        }
    }

    void fill_array() {
        while (arr_size(arr) < 3) {
            char c = 'a';
            ASSERT_EQ(0, arr_insert(arr, arr_size(arr), &c));
        }
    }
};

TEST_F(Array, arr_free) {
    arr_free(0);
    EXPECT_EQ(0, kfree_fake.call_count);

    arr_free(arr);
    EXPECT_EQ(2, kfree_fake.call_count);
    EXPECT_EQ(arr, kfree_fake.arg0_history[1]);

    arr = 0;
}

TEST_F(Array, arr_size) {
    // Empty
    EXPECT_EQ(0, arr_size(arr));

    // 1 element
    char c = '1';
    EXPECT_EQ(0, arr_insert(arr, 0, &c));
    EXPECT_EQ(1, arr_size(arr));

    // 2 element
    c = '2';
    EXPECT_EQ(0, arr_insert(arr, 1, &c));
    EXPECT_EQ(2, arr_size(arr));

    // 3 element
    c = '3';
    EXPECT_EQ(0, arr_insert(arr, 2, &c));
    EXPECT_EQ(3, arr_size(arr));

    void * old_data = arr_data(arr);

    // Grow buffer
    c = '4';
    EXPECT_EQ(0, arr_insert(arr, 3, &c));
    EXPECT_EQ(4, arr_size(arr));

    ASSERT_EQ(1, krealloc_fake.call_count);
    EXPECT_EQ(old_data, krealloc_fake.arg0_val);
    EXPECT_EQ(4, krealloc_fake.arg1_val);
}

TEST_F(Array, arr_data) {
    EXPECT_NE(nullptr, arr_data(arr));
}

TEST_F(Array, arr_at) {
    EXPECT_EQ(0, arr_at(arr, 0));
    EXPECT_EQ(0, arr_at(arr, 1));
    EXPECT_EQ(0, arr_at(arr, 2));
    EXPECT_EQ(0, arr_at(arr, 3));

    char a = '1';
    EXPECT_EQ(0, arr_insert(arr, 0, &a));

    char   b = '1';
    void * c = arr_at(arr, 0);
    EXPECT_NE(0, b);
    EXPECT_EQ('1', *((char *)c));
    EXPECT_EQ(0, arr_at(arr, 1));
    EXPECT_EQ(0, arr_at(arr, 2));

    arr_free(arr);

    arr = arr_new(2, 4);
    ASSERT_NE(nullptr, arr);
    EXPECT_EQ(0, arr_size(arr));

    // No elements
    EXPECT_EQ(0, arr_at(arr, 0));
    EXPECT_EQ(0, arr_at(arr, 1));

    // Read past end
    EXPECT_EQ(0, arr_at(arr, 2));

    int i1 = 12345;
    ASSERT_EQ(0, arr_insert(arr, 0, &i1));

    // 1 element
    int * i2 = (int *)arr_at(arr, 0);
    ASSERT_NE(nullptr, i2);
    EXPECT_EQ(i1, *i2);
    EXPECT_EQ(0, arr_at(arr, 1));

    int i3 = 6789;
    ASSERT_EQ(0, arr_insert(arr, 1, &i3));

    // 2 element
    i2 = (int *)arr_at(arr, 0);
    ASSERT_NE(nullptr, i2);
    EXPECT_EQ(i1, *i2);

    i2 = (int *)arr_at(arr, 1);
    ASSERT_NE(nullptr, i2);
    EXPECT_EQ(i3, *i2);
}

TEST_F(Array, arr_set) {
    char c;

    EXPECT_NE(0, arr_set(0, 0, 0));
    EXPECT_NE(0, arr_set(arr, 0, 0));
    EXPECT_NE(0, arr_set(0, 0, &c));

    c = 'a';
    arr_insert(arr, 0, &c);
    arr_insert(arr, 0, &c);
    arr_insert(arr, 0, &c);

    c = 'b';
    EXPECT_EQ(0, arr_set(arr, 0, &c));
    EXPECT_EQ('b', *(char *)arr_at(arr, 0));

    c = 'c';
    EXPECT_EQ(0, arr_set(arr, 2, &c));
    EXPECT_EQ('c', *(char *)arr_at(arr, 2));
}

TEST_F(Array, arr_get) {
    char c;

    EXPECT_NE(0, arr_get(0, 0, 0));
    EXPECT_NE(0, arr_get(arr, 0, 0));
    EXPECT_NE(0, arr_get(0, 0, &c));

    c = 'a';
    arr_insert(arr, 0, &c);
    c = 'b';
    arr_insert(arr, 0, &c);
    c = 'c';
    arr_insert(arr, 0, &c);

    EXPECT_EQ(0, arr_get(arr, 0, &c));
    EXPECT_EQ('c', c);

    EXPECT_EQ(0, arr_get(arr, 1, &c));
    EXPECT_EQ('b', c);

    EXPECT_EQ(0, arr_get(arr, 2, &c));
    EXPECT_EQ('a', c);

    EXPECT_NE(0, arr_get(arr, 3, &c));
    // Not modified
    EXPECT_EQ('a', c);
}

TEST_F(Array, arr_insert) {
    char c;

    // Invalid parameters
    EXPECT_NE(0, arr_insert(0, 0, 0));
    EXPECT_NE(0, arr_insert(arr, 0, 0));
    EXPECT_NE(0, arr_insert(0, 0, &c));

    EXPECT_EQ(0, arr_size(arr));

    c = '1';
    EXPECT_EQ(0, arr_insert(arr, 0, &c));
    EXPECT_EQ(1, arr_size(arr));

    c = '2';
    EXPECT_EQ(0, arr_insert(arr, 0, &c));
    EXPECT_EQ(2, arr_size(arr));

    c = '3';
    EXPECT_EQ(0, arr_insert(arr, 2, &c));
    EXPECT_EQ(3, arr_size(arr));

    // Grow
    c = '4';
    EXPECT_EQ(0, arr_insert(arr, 3, &c));
    EXPECT_EQ(4, arr_size(arr));

    // Past end
    c = '5';
    EXPECT_NE(0, arr_insert(arr, 6, &c));
    EXPECT_EQ(4, arr_size(arr));

    EXPECT_EQ('2', *(char *)arr_at(arr, 0));
    EXPECT_EQ('1', *(char *)arr_at(arr, 1));
    EXPECT_EQ('3', *(char *)arr_at(arr, 2));
    EXPECT_EQ('4', *(char *)arr_at(arr, 3));
}

TEST_F(Array, array_insert_grow_fails) {
    fill_array();

    krealloc_fake.custom_fake = 0;
    krealloc_fake.return_val  = 0;

    char c = '1';
    EXPECT_NE(0, arr_insert(arr, 0, &c));
}

TEST_F(Array, arr_remove_start_only) {
    char c = 'a';
    arr_insert(arr, 0, &c);
    c = 'b';
    arr_insert(arr, 1, &c);
    c = 'c';
    arr_insert(arr, 2, &c);
    EXPECT_EQ(3, arr_size(arr));
    EXPECT_EQ('a', *(char *)arr_at(arr, 0));
    EXPECT_EQ('b', *(char *)arr_at(arr, 1));
    EXPECT_EQ('c', *(char *)arr_at(arr, 2));

    EXPECT_NE(0, arr_remove(0, 0, 0));
    EXPECT_NE(0, arr_remove(0, 0, &c));
    EXPECT_EQ(3, arr_size(arr));

    EXPECT_EQ(0, arr_remove(arr, 0, &c));
    EXPECT_EQ('a', c);
    EXPECT_EQ(2, arr_size(arr));

    EXPECT_EQ(0, arr_remove(arr, 0, &c));
    EXPECT_EQ('b', c);
    EXPECT_EQ(1, arr_size(arr));

    EXPECT_EQ(0, arr_remove(arr, 0, &c));
    EXPECT_EQ('c', c);
    EXPECT_EQ(0, arr_size(arr));

    EXPECT_NE(0, arr_remove(arr, 0, &c));
    EXPECT_NE(0, arr_remove(arr, 1, &c));

    arr_insert(arr, 0, &c);
    EXPECT_EQ(0, arr_remove(arr, 0, 0));
    EXPECT_EQ(0, arr_size(arr));
}

TEST_F(Array, arr_remove_end_only) {
    char c = 'a';
    arr_insert(arr, 0, &c);
    c = 'b';
    arr_insert(arr, 1, &c);
    c = 'c';
    arr_insert(arr, 2, &c);
    EXPECT_EQ(3, arr_size(arr));
    EXPECT_EQ('a', *(char *)arr_at(arr, 0));
    EXPECT_EQ('b', *(char *)arr_at(arr, 1));
    EXPECT_EQ('c', *(char *)arr_at(arr, 2));

    EXPECT_NE(0, arr_remove(0, 0, 0));
    EXPECT_NE(0, arr_remove(0, 0, &c));
    EXPECT_EQ(3, arr_size(arr));

    EXPECT_EQ(0, arr_remove(arr, 2, &c));
    EXPECT_EQ('c', c);
    EXPECT_EQ(2, arr_size(arr));

    EXPECT_EQ(0, arr_remove(arr, 1, &c));
    EXPECT_EQ('b', c);
    EXPECT_EQ(1, arr_size(arr));

    EXPECT_EQ(0, arr_remove(arr, 0, &c));
    EXPECT_EQ('a', c);
    EXPECT_EQ(0, arr_size(arr));

    EXPECT_NE(0, arr_remove(arr, 0, &c));
    EXPECT_NE(0, arr_remove(arr, 1, &c));
}
